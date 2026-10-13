# unikernel-build

## License

The files in this repository are licensed under the [Apache License 2.0](LICENSE).

That license does not extend to:

- The submodules (`mirage-skeleton`, `unikernel-httpreply`, `unikernel-redis`), which keep their own licenses.
- The software inside the published images:
  - The Linux kernels are licensed under `GPL-2.0-only WITH Linux-syscall-note`. Each is built from an unmodified [kernel.org](https://kernel.org) release, using the defconfig under `kernel/` and `.github/workflows/kernels.yml`. Each `linux-kernel` image ships the license and the expanded `.config` under `/.boot/`. Its labels name the exact source tarball and its SHA-256 (`com.urunc.kernel.tarball`, `com.urunc.kernel.tarball.sha256`) and the defconfig it was built from (`com.urunc.kernel.defconfig`).
  - The unikernel frameworks, applications and root filesystems in the images are under their own licenses.
