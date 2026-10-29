# hector_ros2_ci

Reusable GitHub Actions CI for Hector ROS 2 packages: lint, build and test against prebuilt dependencies from [deb.teamhector.de](http://deb.teamhector.de).

No `dependencies.repos`, no `vcs import`: only your repository is built from source. Everything else is installed as binary packages via `rosdep`.

## Usage

There are two reusable workflows. Use one or both:

| Workflow             | Does                                                    |
| -------------------- | ------------------------------------------------------- |
| `pre-commit.yml`     | Runs [pre-commit](https://pre-commit.com) on all files (needs a `.pre-commit-config.yaml`). |
| `build-and-test.yml` | Installs dependencies with `rosdep`, then `colcon build` and `colcon test`. |

Add `.github/workflows/ci.yml` to your repository:

```yaml
name: CI

on:
  push:
    branches: [jazzy]
  pull_request:

jobs:
  pre-commit:
    uses: tu-darmstadt-ros-pkg/hector_ros2_ci/.github/workflows/pre-commit.yml@v1
  build-and-test:
    uses: tu-darmstadt-ros-pkg/hector_ros2_ci/.github/workflows/build-and-test.yml@v1
```

Lint only? Just remove the `build-and-test` job (and vice versa).

Make sure all dependencies are declared in your `package.xml` files.

### Options for `build-and-test.yml`

| Input       | Default        | Description                                                                 |
| ----------- | -------------- | --------------------------------------------------------------------------- |
| `rosdistro` | `lyrical`      | ROS 2 distribution, or a JSON list of several (`'["jazzy", "lyrical"]'`).   |
| `runner`    | `ubuntu-24.04` | Runner label, e.g. `ubuntu-24.04-arm` for arm64.                            |

With a list, each distribution runs as its own job in parallel (a matrix); one failing distribution does not cancel the others.

```yaml
  build-and-test:
    uses: tu-darmstadt-ros-pkg/hector_ros2_ci/.github/workflows/build-and-test.yml@v1
    with:
      rosdistro: '["jazzy", "lyrical"]'
      runner: ubuntu-24.04-arm
```

## What `build-and-test` does

In a `ros:<rosdistro>-ros-base` container it

1. adds the Hector apt repository and rosdep rules,
2. runs `rosdep install` for all packages in the repository,
3. runs `colcon build`, `colcon test` and `colcon test-result`.

Hector packages are installed to `/opt/hector/<rosdistro>` and take precedence over `/opt/ros/<rosdistro>`, just like in the Hector workspace.

## Good to know

- **Dependencies come from the latest released packages** on the build server, not from source. If you change two repositories at once (e.g. a `*_msgs` package and its user), the second one can only pass after the build server has published the first.
- **`Cannot locate rosdep definition for [xyz]`** means `xyz` is neither in rosdistro nor on the Hector build server. Add the package to the build server, or fix the name in `package.xml`.
- Both workflows need no secrets and only read access.

## Maintenance

Changes are tested by [ci.yml](.github/workflows/ci.yml), which runs both workflows from the current branch on this repository (building the smoke package in [test/](test/hector_ci_smoke)). Run `pre-commit install` to lint locally.

Callers use the `v1` tag. After merging a change to `main` with green CI, move the tag:

```bash
git tag -f v1 && git push -f origin v1
```

Use a new major tag (`v2`) for changes that require callers to adapt.
