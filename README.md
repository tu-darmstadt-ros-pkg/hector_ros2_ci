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

| Input        | Default          | Description                                                               |
| ------------ | ---------------- | ------------------------------------------------------------------------- |
| `rosdistro`  | `lyrical`        | ROS 2 distribution, or a JSON list of several (`'["jazzy", "lyrical"]'`). |
| `runner`     | `ubuntu-24.04`   | Runner label, e.g. `ubuntu-24.04-arm` for arm64.                          |
| `build-type` | `RelWithDebInfo` | `CMAKE_BUILD_TYPE` of the build, e.g. `Debug`.                            |
| `cmake-args` |                  | Extra CMake arguments in shell syntax, quote values with spaces.          |
| `ctest-args` |                  | Arguments for `ctest`, e.g. `-j$(nproc)` to run tests in parallel or `-L <label>` for a subset. |
| `clang-tidy` | `false`          | Run `clang-tidy` on the repository's sources after the build. Any finding fails the job. |

With a list, each distribution runs as its own job in parallel (a matrix); one failing distribution does not cancel the others.

```yaml
  build-and-test:
    uses: tu-darmstadt-ros-pkg/hector_ros2_ci/.github/workflows/build-and-test.yml@v1
    with:
      rosdistro: '["jazzy", "lyrical"]'
      runner: ubuntu-24.04-arm
```

Call the workflow twice for a second configuration, e.g. a subset of the tests under AddressSanitizer next to the normal build. Both jobs run in parallel:

```yaml
  asan:
    uses: tu-darmstadt-ros-pkg/hector_ros2_ci/.github/workflows/build-and-test.yml@v1
    with:
      cmake-args: '"-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer"'
      ctest-args: -j$(nproc) -L concurrency
```

`clang-tidy` reads the `.clang-tidy` files of the repository, without one it runs its default checks. Tests running in parallel must not share a ROS domain, see `ament_add_ros_isolated_gtest` in `ament_cmake_ros`.

## What `build-and-test` does

In a `ros:<rosdistro>-ros-base` container it

1. adds the Hector apt repository and rosdep rules,
2. installs `hector_workspace` and runs `rosdep install` for all packages in the repository,
3. runs `colcon build`, optionally `clang-tidy`, then `colcon test` and `colcon test-result`.

Builds go through [ccache](https://ccache.dev). Its cache is kept per distribution, architecture and configuration (`build-type` and `cmake-args`), so a second push to a pull request compiles only what changed. Pull requests start from the cache of their base branch.

Hector packages are installed to `/opt/hector/<rosdistro>` and take precedence over `/opt/ros/<rosdistro>`, just like in the Hector workspace. Build and test source `/opt/hector/<rosdistro>/setup.bash`, so package environment hooks apply.

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
