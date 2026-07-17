# unikernel-build

Build (and boot-test) unikernel and single-application-kernel OCI images for
[urunc](https://github.com/urunc-dev/urunc).

## Repo layout

Images are described by [bunny](https://github.com/nubificus/bunny) files laid
out as:

```
<app>/<framework>/<monitor>/bunnyfile-<arch>
```

e.g. `hello-world/unikraft/qemu/bunnyfile-amd64`. The CI matrix is derived
from this layout: a combination is built only if its bunnyfile exists. (The
unikraft catalog has no firecracker/arm64 variants, so e.g.
`hello-world/unikraft/firecracker/` intentionally has no `bunnyfile-arm64`.)

Images are pushed to `ghcr.io/urunc-dev/unikernel-build/<app>-<monitor>-<framework>`
with tags `<arch>-<commit sha>` plus a multi-arch manifest tagged `<commit sha>`.

The top-level `nginx/`, `redis/`, `rumprun-*/`, `*-mirage/` directories are the
legacy flat layout, still consumed by the legacy `oci.yml` workflow. They will
be migrated to the layout above.

## Workflows

Everything is **manual (`workflow_dispatch`) for now** so each job can be
debugged in isolation. Once the matrix is stable, `build_all.yml` (or
`build_changed_images.yml`) gets a push trigger back.

Entry points:

| Workflow | Trigger | What it does |
|---|---|---|
| `manual-build.yml` | dispatch | Build ONE (app, framework, monitor, arch) combo; `arch: all` also creates the manifest; optional urunc boot test. |
| `test-boot.yml` | dispatch or call | Boot an already-pushed image with urunc on a GitHub runner and assert expected console output. |
| `build_all.yml` | dispatch | Discover every bunnyfile in the repo and build + manifest + boot-test the full matrix. |
| `build_changed_images.yml` | dispatch | Diff-driven variant: build only combos whose files changed vs `base`. |
| `oci.yml` | dispatch | Legacy monolithic pipeline over the old flat layout. |

Reusable plumbing (workflow_call only): `separate_frameworks.yml` →
`separate_monitors.yml` → `build_and_manifest.yml` → `build_images.yml` /
`test-boot.yml`.

All entry points and the full `build_all.yml` matrix (builds, multi-arch
manifests, urunc boot tests) have been dispatch-verified green on this layout.

Examples:

```sh
# build hello-world/unikraft/qemu for amd64 and boot-test it
gh workflow run manual-build.yml -f app=hello-world -f framework=unikraft \
  -f monitor=qemu -f arch=amd64 -f boot_test=true

# boot-test an existing image
gh workflow run test-boot.yml \
  -f image=ghcr.io/urunc-dev/unikernel-build/hello-world-qemu-unikraft:amd64-<sha> \
  -f monitor=qemu

# full matrix
gh workflow run build_all.yml -f boot_test=true
```

### Dispatching from a branch (workflow development tip)

GitHub only registers `workflow_dispatch`-only workflows from the default
branch. To debug a new dispatchable workflow on a feature branch before it
lands there, temporarily add a `push` trigger for that branch (guard the jobs
to no-op on push) — the push registers the workflow. It can then be
dispatched by numeric ID with an explicit ref; remove the trigger before
merging:

```sh
gh api repos/<owner>/<repo>/actions/workflows --jq '.workflows[] | "\(.id) \(.path)"'
gh workflow run <workflow-id> --ref <branch> -f app=hello-world ...
```

## Building

`build_images.yml` builds each bunnyfile with docker/build-push-action on a
runner matching the target architecture (`ubuntu-22.04` / `ubuntu-22.04-arm`).

The buildkit daemon is **pinned to v0.30.0**: buildkit v0.31 changed platform
matching so that a manifest entry's `os.features` must be a subset of the
requested platform's, and unikraft catalog images carry their full kconfig in
`os.features` — on a v0.31+ daemon (the current `buildx-stable-1`) resolving
`kernel.from: unikraft.org/...` fails with `no match for platform in manifest:
not found`, regardless of bunny version. Keep the pin until this is resolved
in buildkit/kraftkit.

## Boot testing

`test-boot.yml` installs containerd, nerdctl, CNI plugins, the target monitor
(qemu or firecracker) and the urunc static release binaries on an
`ubuntu-22.04` runner (KVM is available there), then runs:

```sh
nerdctl run --rm --runtime io.containerd.urunc.v2 <image>
```

and greps the console output for `expected_output` (default
`Hello from Unikraft!`). The default overlayfs snapshotter is sufficient for
the current images (no block rootfs); devmapper setup can be added when
images with `mountRootfs=true` join the matrix.

Boot tests run on amd64 only for now: GitHub arm64 runners do not provide KVM.

When testing manually on a dev host, note that `/etc/urunc/config.toml` may
pin monitor binary paths (e.g. `monitors.firecracker.path =
"/opt/urunc/bin/firecracker"`); the binary must exist at that exact path or
urunc fails with a bare `no such file or directory` at start.
