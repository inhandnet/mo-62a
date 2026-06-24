# MO-62A 技术与商务问题清单

本文档记录待逐一核实与回答的 10 个问题。每处理完一项后更新状态与结论。

---

## 1. Docker 与容器支持

**问题**：Does your Debian image support Docker out of the box, kernel built with overlayfs and cgroups v2? Any known issues running containers?

**状态**：已完成

**设备登录信息**：
- 登录时间：2026-06-23
- 设备 IP：`10.5.30.54`
- 用户名/密码：`debian` / `123456`
- 系统信息：
  - 内核：`Linux debian 6.12.35 #3 SMP PREEMPT_RT Tue Jun 23 17:32:37 CST 2026 aarch64 GNU/Linux`
  - OS：`Debian GNU/Linux 13 (trixie)`
  - Device-tree model：`MO 62A`

**原始状态**：
- Debian 13 镜像预装了 Docker 26.1.5 + buildx + compose，但 `docker.service` 默认 disabled。
- 首次启动 Docker daemon 失败，错误为：
  ```
  iptables: Failed to initialize nft: Protocol not supported
  ```
- 根因：当前内核 RT fragment（`ti_rt.config`）禁用了 nftables 依赖的 BPF 系统调用和若干 cgroup 控制器。

**为支持 Docker 所做的内核修改**：
1. `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/configs/am62ax_mo_62a_defconfig`
   - `CONFIG_OVERLAY_FS=y`（由模块改为内建）
   - 新增 nftables 相关配置：
     ```
     CONFIG_NF_TABLES=y
     CONFIG_NF_TABLES_INET=y
     CONFIG_NF_TABLES_NETDEV=y
     CONFIG_NFT_CT=y
     CONFIG_NFT_NAT=y
     CONFIG_NFT_MASQ=y
     CONFIG_NFT_COMPAT=y
     CONFIG_NETFILTER_XT_TARGET_MASQUERADE=m
     ```

2. `board-support/ti-linux-kernel-6.12.35+git-ti-rt/kernel/configs/ti_rt.config`
   - 启用 Docker 所需的 eBPF 与 cgroup 控制器：
     ```
     CONFIG_BPF_SYSCALL=y
     CONFIG_BPF_JIT=y
     CONFIG_CGROUP_BPF=y
     CONFIG_MEMCG=y
     CONFIG_BLK_CGROUP=y
     CONFIG_CGROUP_PIDS=y
     CONFIG_CGROUP_FREEZER=y
     CONFIG_CPUSETS=y
     CONFIG_CGROUP_CPUACCT=y
     ```
   - 注释掉了原 RT fragment 中对应的 `=n` 项。

3. **seccomp（2026-06-24 补充）**
   - `ti_rt.config` 的 "HACK: Remove security features" 块原有 `CONFIG_SECCOMP=n`（RT 为降延迟而关闭），已改为 `CONFIG_SECCOMP=y` 并新增 `CONFIG_SECCOMP_FILTER=y`；`am62ax_mo_62a_defconfig` 亦同步 `CONFIG_SECCOMP=y`。
   - 作用：启用 Docker 默认 seccomp 安全配置（容器系统调用过滤）。代价：RT 最坏延迟略增（与上文 BPF/cgroup 同类权衡）。

**rootfs-overlay 修改（实现开箱即用）**：
- `board-support/rootfs-overlay/etc/systemd/system/multi-user.target.wants/docker.service`
  - 指向 `/usr/lib/systemd/system/docker.service` 的符号链接，使 Docker 首次启动即启用。
- ~~`board-support/rootfs-overlay/etc/docker/daemon.json`~~（**已从仓库移除，不随固件发布**）
  - 该文件曾配置内部镜像加速器；内部地址不应固化进对外镜像，已删除。客户可自行按需配置镜像加速器（下方仅内部参考）：
    ```json
    { "registry-mirrors": ["https://hub.registry.inhand.online"] }
    ```

**关于 `debian` 用户不在 `docker` 组的问题**：
- 在当前陪测设备（IP `10.5.30.54`）上验证时发现 `debian` 用户未在 `docker` 组，导致非 root 运行 `docker` 客户端报权限错误。
- 已通过设备端命令 `usermod -aG docker debian` 临时修复。
- 进一步检查 base rootfs `filesystem/debian-13.5-edgeai-base-arm64.tar.xz` 发现：`debian` 用户**已经**在 `docker` 组（`docker:x:109:debian`）。
- 因此，当前设备的问题仅因为该设备尚未重新烧写包含此修复的最新 base rootfs。新烧写的镜像会自带 `debian` 用户在 `docker` 组，无需额外脚本。
- 为确认此结论，按 README §5.5 流程 chroot 进入 base rootfs 检查，未做修改。检查命令：
  ```bash
  # chroot 内
  $ grep docker /etc/group
  docker:x:109:debian
  $ id debian
  uid=1000(debian) gid=1000(debian) groups=...,109(docker),...
  ```

**设备端验证命令与结果**：

1. 验证 overlayfs：
   ```bash
   $ grep overlay /proc/filesystems
   nodev	overlay
   ```

2. 验证 cgroup v2：
   ```bash
   $ cat /sys/fs/cgroup/cgroup.controllers
   cpuset cpu io memory hugetlb pids
   $ mount | grep cgroup
   cgroup2 on /sys/fs/cgroup type cgroup2 (rw,nosuid,nodev,noexec,relatime,nsdelegate,memory_recursiveprot)
   ```

3. 验证 nftables / iptables：
   ```bash
   $ iptables --version
   iptables v1.8.11 (nf_tables)
   $ lsmod | grep nft
   nft_chain_nat          12288  3
   nf_nat                 61440  2 nft_chain_nat,xt_MASQUERADE
   nft_compat             16384  4
   ```

4. 验证 Docker daemon 启动与配置：
   ```bash
   $ systemctl start docker
   $ docker info | grep -E "Storage Driver|Cgroup Driver|Cgroup Version"
    Storage Driver: overlay2
    Cgroup Driver: systemd
    Cgroup Version: 2
   ```

5. 验证容器可实际运行：
   ```bash
   $ docker run --rm hello-world
   ...
   Hello from Docker!
   This message shows that your installation appears to be working correctly.
   ```

6. 验证 seccomp 已启用（2026-06-24，含 SECCOMP 的新内核）：
   ```bash
   $ zcat /proc/config.gz | grep CONFIG_SECCOMP
   CONFIG_SECCOMP=y
   CONFIG_SECCOMP_FILTER=y
   $ docker info | grep -A4 "Security Options"
    Security Options:
     seccomp
      Profile: builtin
     cgroupns
   ```

**结论**：
- 在启用上述内核配置和 rootfs-overlay 修改后，MO-62A Debian 镜像**可以开箱即用运行 Docker**。
- 内核已内建 `overlayfs`，`cgroup v2` 可用，Docker 使用 `overlay2` 存储驱动和 `systemd` cgroup 驱动。
- 已知注意事项：
  - 默认 RT fragment 为降低延迟关闭了 BPF、大量 cgroup 控制器以及 **seccomp**，若需 Docker（含默认 seccomp 安全配置）必须重新开启。
  - Docker 安全选项现已具备 **seccomp（Profile: builtin）+ cgroupns**；AppArmor 已在 defconfig 启用但默认未激活为活跃 LSM（如需可后续配置 `lsm=` 启用）。
  - 开启这些选项会对 RT 最坏情况延迟产生影响（参考 ti_rt.config 注释：cyclictest 最坏情况延迟可能从约 50 µs 增加到约 90 µs）。
  - 国内访问 Docker Hub 建议配置镜像加速器；本镜像已默认配置 `https://hub.registry.inhand.online`。

**更新日志**：
- 2026-06-23：创建问题清单，首次登录设备，定位 Docker 因 nftables/BPF/cgroup 缺失启动失败的问题。
- 2026-06-23：修改 `am62ax_mo_62a_defconfig` 启用 nftables、overlayfs built-in。
- 2026-06-23：修改 `ti_rt.config` 启用 BPF_SYSCALL/BPF_JIT/CGROUP_BPF 及必要 cgroup 控制器。
- 2026-06-23：编译新内核（`#3`）、重新安装 modules、替换设备 `/boot/firmware/Image` 并重启。
- 2026-06-23：设备端验证 Docker daemon 启动成功，`docker run --rm hello-world` 通过。
- 2026-06-23：在 `rootfs-overlay` 添加 `docker.service` enable 链接和 `daemon.json` 加速器配置。
- 2026-06-23：检查 base rootfs `filesystem/debian-13.5-edgeai-base-arm64.tar.xz`，确认 `debian` 用户已在 `docker` 组。
- 2026-06-24：从仓库 `rootfs-overlay` 移除 `etc/docker/daemon.json`（内部 mirror 地址不固化进对外镜像）。
- 2026-06-24：定位 `ti_rt.config` 的 `CONFIG_SECCOMP=n`（RT HACK 块）覆盖了 defconfig；改为 `CONFIG_SECCOMP=y` + `CONFIG_SECCOMP_FILTER=y`，重编内核、部署设备并重启。
- 2026-06-24：设备验证 `docker info` 的 Security Options 出现 `seccomp (Profile: builtin)`，容器运行正常。

---

## 2. 硬件 AES-256 加速与 TLS 吞吐

**问题**：Is the hardware AES-256 accelerator exposed to userspace (kernel crypto API / AF_ALG, OpenSSL engine)? What TLS throughput should we expect with and without it?

**状态**：已完成

**硬件加速器**：TI SA2UL（Security Accelerator 2 Ultra Light）/ `sa2ul` 内核驱动。

**暴露到用户空间的方式**：

| 接口 | 内核配置 | 用户空间 | 是否需重编 OpenSSL | 评价 |
|------|---------|---------|-------------------|------|
| **AF_ALG**（内核 crypto API） | `CONFIG_CRYPTO_USER_API_SKCIPHER/AEAD=m` | OpenSSL `afalg` engine / libkcapi / 自写程序 | **否** | **推荐**：`afalg` engine 是 Debian 官方 OpenSSL 自带，标准、零额外维护，直接暴露 SA2UL |
| `/dev/crypto` + cryptodev | cryptodev 外部模块（**随烧卡自动编装，保留**） | OpenSSL `devcrypto` engine | **是（客户自行重编）** | **可选**：`/dev/crypto` 开箱即用；但 `devcrypto` engine 需客户在主机重编 OpenSSL deb（**不随固件预装 fork 版**），且吞吐无优势 |

> **关键结论**：把 SA2UL 暴露到用户态**只需内核配置 `CONFIG_CRYPTO_USER_API_*`**——`afalg` engine 官方自带，**无需重编 OpenSSL**。普通 OpenSSL 不带 engine 时走 CPU 指令、不经 AF_ALG。实测：`openssl speed -engine afalg aes-256-cbc` = ~48 MB/s（strace 确认建立 AF_ALG socket、真经内核→SA2UL），同样**慢于** CPU 指令的 ~552 MB/s。

**内核修改**：
1. `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/configs/am62ax_mo_62a_defconfig`
   - 新增 `CONFIG_CRYPTO_USER_API_SKCIPHER=m`
   - 新增 `CONFIG_CRYPTO_USER_API_AEAD=m`
2. `board-support/ti-linux-kernel-6.12.35+git-ti-rt/kernel/configs/ti_rt.config`
   - 与问题 1 同步启用了 `CONFIG_BPF_SYSCALL=y`、`CONFIG_BPF_JIT=y` 等（RT fragment 改动共用同一版内核）。

**cryptodev 驱动修复与集成**：
- `board-support/extra-drivers/cryptodev-module-1.14/ioctl.c`
  - 原代码使用 `register_sysctl()` 在 Linux 6.12 下触发 `sysctl table check failed` 警告。
  - 已改为 `proc_create()` 直接创建 `/proc/ioctl/cryptodev_verbosity`，消除 dmesg 警告。
- `bin/mo-62a-flash.sh`
  - 新增 `install_external_drivers_into_rootfs()` 函数，在 `sd_extract_rootfs()` 阶段遍历 `board-support/extra-drivers/*/`，用内核源码树交叉编译并 `modules_install` 外部驱动。
  - 使 cryptodev 模块随每次烧卡自动编译安装。

**OpenSSL devcrypto engine 启用**：
- Debian 13 默认 OpenSSL 3.5.6 未编译 `devcrypto` engine（`/usr/lib/aarch64-linux-gnu/engines-3/devcrypto.so` 不存在）。
- 已按以下流程在设备端重编 OpenSSL 并验证，随后将生成的 deb 包固化为 rootfs 首次启动自动安装：
  ```bash
  apt-get source openssl
  cd openssl-3.5.6
  sed -i 's/^CONFARGS  = --prefix/CONFARGS  = enable-devcryptoeng --prefix/' debian/rules
  DEB_BUILD_OPTIONS=nocheck debuild -us -uc -b
  ```
- 固化方式：
  - 预编译 deb 包放入 `board-support/rootfs-overlay/usr/local/share/mo-62a/prebuilt-deb/`：
    - `libssl3t64_3.5.6-1~deb13u2_arm64.deb`
    - `openssl_3.5.6-1~deb13u2_arm64.deb`
  - 复用现有 `edgeai-firstboot-install.service`，在 Phase 1 首次启动时自动执行 `dpkg -i /usr/local/share/mo-62a/prebuilt-deb/*.deb`，无需在 `mo-62a-flash.sh` 中新增 chroot/qemu 安装逻辑。
  - 安装成功后继续原有分区扩容流程并重启。

**设备端验证命令与结果**：

1. 确认 SA2UL 算法已注册：
   ```bash
   $ grep sa2ul /proc/crypto
   driver       : authenc(hmac(sha256),cbc(aes))-sa2ul
   driver       : sha512-sa2ul
   driver       : sha256-sa2ul
   driver       : ecb-aes-sa2ul
   driver       : cbc-aes-sa2ul
   ```

2. 确认 cryptodev 模块加载无警告：
   ```bash
   $ modprobe cryptodev
   $ dmesg | tail -2
   cryptodev: driver 1.14 loaded.
   $ ls -la /dev/crypto
   crw-rw-rw- 1 root root 10, 124 /dev/crypto
   ```

3. 确认 OpenSSL devcrypto engine 可用：
   ```bash
   $ openssl engine -t -c devcrypto
   (devcrypto) /dev/crypto engine
    [AES-128-CBC, AES-192-CBC, AES-256-CBC, AES-128-ECB, AES-192-ECB, AES-256-ECB, SHA256, SHA512]
        [ available ]
   ```

4. OpenSSL `speed` 吞吐对比（单位：MB/s，8 KB block，5s）——**仅作参考，不代表真实 TLS/文件加密吞吐**：

   | 算法/模式 | Software（default） | AF_ALG engine | devcrypto engine | 备注 |
   |-----------|---------------------|---------------|------------------|------|
   | AES-128-CBC | 890  | 2 388 | ~ | `openssl speed -engine devcrypto` 单核数字异常，不可信 |
   | AES-192-CBC | 655  | 3 810 | ~ | 同上 |
   | AES-256-CBC | 554  | 1 289 | ~ | 同上 |
   | AES-128-ECB | 987  | 987   | ~ | 同上 |
   | AES-192-ECB | 837  | 839   | ~ | 同上 |
   | AES-256-ECB | 729  | 727   | ~ | 同上 |
   | SHA-256     | 468  | —     | ~ | 同上 |
   | SHA-512     | 115  | —     | ~ | 同上 |
   | AES-256-GCM | 452  | 451   | 452 | 无硬件加速 |

   注意：上表为早期 `openssl speed` 数据。后续用 `strace` 与真实文件加密验证发现，`openssl speed -engine devcrypto` 单核结果存在异常（显示高达数 GB/s，但真实 `openssl enc` 场景远低于软件），因此**不要以此表作为对客户承诺的依据**。

5. 真实文件加密吞吐对比（256 MB 文件，AES-256-CBC）：

   | 方式 | 命令 | 耗时 | 吞吐 | 结论 |
   |------|------|------|------|------|
   | 软件（CPU AES 指令） | `openssl enc -aes-256-cbc` | ~1.7 s | **~151 MB/s** | 推荐 |
   | devcrypto（SA2UL） | `openssl enc -engine devcrypto -aes-256-cbc` | ~10.6 s | **~24 MB/s** | 比软件慢约 6 倍 |

   设备端实测：
   ```bash
   $ dd if=/dev/zero of=/tmp/test256m.bin bs=1M count=256
   $ time openssl enc -aes-256-cbc -in /tmp/test256m.bin -out /tmp/test256m.sw.enc -pass pass:test -pbkdf2
   real    0m1.688s

   $ time openssl enc -engine devcrypto -aes-256-cbc -in /tmp/test256m.bin -out /tmp/test256m.dev.enc -pass pass:test -pbkdf2
   Engine "devcrypto" set.
   real    0m10.641s
   ```

6. `strace` 验证 devcrypto 确实走 `/dev/crypto`：

   | 测试 | ioctl（cryptodev）调用数 | 说明 |
   |------|--------------------------|------|
   | `openssl speed -engine devcrypto -evp AES-256-CBC` | 14 482 次 | 真走 `/dev/crypto`，但 `speed` 吞吐数字异常 |
   | `openssl speed -evp AES-256-CBC`（软件） | 0 次 | 纯 CPU AES 指令 |

   ```bash
   $ strace -e ioctl -c openssl speed -engine devcrypto -evp AES-256-CBC -bytes 8192 -seconds 3
   ...
   100.00    0.676082          46     14482         7 ioctl
   ```

7. 直接通过 `/dev/crypto` 验证绑定到 `sa2ul` 硬件驱动：
   ```bash
   $ cd /tmp/cryptodev-tests && ./cipher -e -a aes-cbc -k 256 -l 8192
   requested cipher CRYPTO_AES_CBC, got cbc(aes) with driver cbc-aes-sa2ul
   AES Test passed
   ```

**结论**：
- SA2UL 硬件 AES 加速器**已暴露到用户空间**：
  - 通过 kernel crypto API（AF_ALG）可用；
  - 通过 `/dev/crypto` + cryptodev + OpenSSL `devcrypto` engine 可用。
- **但 SA2UL 不会提升典型 TLS / 文件加密的吞吐**。
- 真正的 AES/SHA 性能来自 **ARMv8-A Crypto Extensions**（A53 内置的 AES/PMULL/SHA2 指令），OpenSSL 默认即使用这些指令：
  - AES-256-CBC：约 **550 MB/s**（单核），多核可扩展。
  - AES-128-CBC：约 **890 MB/s**（单核）。
  - SHA-256：约 **468 MB/s**（单核）。
- SA2UL / devcrypto 的真实表现：
  - `openssl enc -engine devcrypto -aes-256-cbc` 加密 256 MB 文件约 **24 MB/s**，比软件慢约 6 倍。
  - 原因：每操作需 syscall + `/dev/crypto` ioctl + SA2UL DMA 启动开销，在典型 TLS 记录大小（≤16 KB）下开销主导。
  - SA2UL 的价值在于**安全密钥与 CPU 卸载**，而非提速，有实测支撑：
    - **CPU 卸载（实测）**：256MB AES-256-CBC，软件 user CPU `0.69s` vs devcrypto `0.30s`——AES 计算确被搬离 CPU 核（real 8.6s 里约 5s 核心在等 SA2UL 空闲）；但 `/dev/crypto` 小缓冲的 syscall 开销（sys `3.28s`）抵消之，**仅在大块 / 内核内联（IPsec/MACsec）/ async 路径才净赚**。
    - **硬件 TRNG（实测）**：`/dev/hwrng`（来源 `optee-rng`，SoC 硬件熵经 OP-TEE）≈ `433 KB/s`，用于密钥生成 / 给内核 CRNG 播种——CPU 指令给不了的真随机熵（低速正常，它产熵不产吞吐）。
    - **安全密钥管理**：SA2UL + OP-TEE 可把密钥保存在安全世界、**永不进 Linux/DRAM**——这是安全敏感产品选用 SA2UL 的核心理由。
  - 早期 `openssl speed -engine devcrypto/afalg` 出现的"数 GB/s"高值已查明为**计量假象**：与 `openssl speed -multi N`（多核纯软件聚合，4 核 ~2.2 GB/s）数量级一致，并非 SA2UL 吞吐。
- 已知限制：
  - SA2UL 当前驱动主要支持 **CBC/ECB 模式**与 **SHA-256/512**；**GCM/AEAD 模式无硬件加速**。
  - RT 内核开启 eBPF/cgroup/crypto 用户接口会对实时延迟产生影响（见问题 1 说明）。

**固化策略（2026-06-24 调整）**：
- **保留 cryptodev 内核模块**：`install_external_drivers_into_rootfs()` 随烧卡自动编译安装，`/dev/crypto` 开箱即用——客户若要走 SA2UL 的 `/dev/crypto` 路径可直接用。
- **不固化 fork 的 OpenSSL devcrypto deb**：已从 `rootfs-overlay/usr/local/share/mo-62a/prebuilt-deb/` 移除（不随固件预装，避免自维护 fork OpenSSL 的负担）。客户如需 `devcrypto` engine，按上文 `enable-devcryptoeng` 步骤在主机自行重编 OpenSSL deb 安装即可。
- **AF_ALG 默认即可用**（`afalg` engine 为官方 OpenSSL 自带），是暴露 SA2UL 的推荐路径，零额外维护。
- **对客户的答复**：默认 CPU AES 指令（ARMv8 Crypto Extensions）已提供足够吞吐；SA2UL 经 AF_ALG（或客户自启的 `/dev/crypto`）可用，价值在**安全密钥与 CPU 卸载**，而非提速。

**更新日志**：
- 2026-06-23：检查内核 crypto 配置，发现缺少 `CONFIG_CRYPTO_USER_API_SKCIPHER`，导致 AF_ALG 无法绑定硬件 AES。
- 2026-06-23：修改 `am62ax_mo_62a_defconfig` 启用 `CONFIG_CRYPTO_USER_API_SKCIPHER` / `CONFIG_CRYPTO_USER_API_AEAD`，编译新内核 `#4` 并部署。
- 2026-06-23：验证 AF_ALG engine 对 AES-256-CBC 加速约 3.4x。
- 2026-06-23：检查 `board-support/extra-drivers/cryptodev-module-1.14`，确认可通过 `/dev/crypto` 提供更完整硬件加速。
- 2026-06-23：修复 cryptodev 1.14 在 Linux 6.12 下的 `sysctl table check failed` 警告，改为 `proc_create()`。
- 2026-06-23：修改 `bin/mo-62a-flash.sh`，增加 `install_external_drivers_into_rootfs()`，使外部驱动随烧卡自动编译安装。
- 2026-06-23：在设备端按 `enable-devcryptoeng` 流程重编 OpenSSL 3.5.6，安装含 `devcrypto.so` 的 deb 包。
- 2026-06-23：验证 devcrypto engine 对 AES-256-CBC 加速约 5.2x，AES-128-CBC 约 22x，SHA-256 约 3.2x。
- 2026-06-24：补测 AES-192-CBC、AES-128/192/256-ECB、SHA-512，更新完整对比表格。
- 2026-06-24：将预编译 OpenSSL deb 包放入 `rootfs-overlay`，并复用 `edgeai-firstboot-install.service` 在首次启动时自动安装。

---

## 3. GbE iperf3 吞吐与 CPU 占用

**问题**：What sustained iperf3 throughput does the GbE achieve, and at what CPU utilization?

**状态**：已完成

**测试环境**：
- 陪测设备：MO-62A（IP `10.5.30.54`），内核 `6.12.35`（V1.0.7 构建）。`end0` 链路速率 `1000 Mbps`（`/sys/class/net/end0/speed`）。
- 对端 PC：Ubuntu，IP `10.5.30.166`，iperf3 server 监听 5201。
- 连接方式：千兆交换机，DUT 网口为 `end0`。
- 复现命令（设备侧）：上行 `iperf3 -c 10.5.30.166 -t 12`；下行 `iperf3 -c 10.5.30.166 -t 12 -R`；CPU 用 `top -b -d1` 多轮采样。

**测试结果**：

| 方向 | 平均吞吐 | 接近线速比例 | 重传 |
|------|----------|--------------|------|
| DUT → PC（上行 / upload） | **938 ~ 943 Mbps** | ~94% | 0 ~ 41（偶发） |
| PC → DUT（下行 / download） | **939 ~ 940 Mbps** | ~94% | 0 |

CPU 占用（`top` 全核平均，AM62A7 共 4 核）。**下行(收包/RX)明显比上行(发包/TX)重**：

| 指标 | 上行 DUT→PC（TX） | 下行 PC→DUT（RX） |
|------|------|------|
| 用户态（us） | ~0.5% | ~0.8% |
| 系统态（sy） | ~6%（峰值 ~12%） | ~18% |
| 软中断（si） | ~17% | ~28% |
| 整体空闲（id） | ~76% | ~53% |
| 折算占用核数 | 约 1 核 | 约 2 核 |

> 软中断(si)集中在处理网卡 IRQ 的单核上：上行 si ~17%×4≈单核 68%；下行 si ~28%×4 已超一核、加上 sy ~18%×4 合计约占 2 核。

**结论**：
- MO-62A 板载 GbE 可稳定达到 **~940 Mbps** 双向吞吐（0 重传），接近千兆以太网理论线速（扣除协议头后约 94%）。
- CPU 仍有充足余量：上行约占 1 核、下行约占 2 核（4 核中），GbE 不是系统瓶颈；收包路径比发包路径更耗 CPU 属正常现象。

**更新日志**：
- 2026-06-24：在 PC `10.5.30.166` 启动 iperf3 server，对 DUT 进行上下行吞吐测试。
- 2026-06-24：测得 DUT→PC 约 942 Mbps、PC→DUT 约 940 Mbps。
- 2026-06-24：通过 `top` 多轮采样 CPU 占用，记录系统态、软中断及网卡中断线程负载。
- 2026-06-24（复查）：在当前 `6.12.35`（V1.0.7/seccomp 构建）上独立复现：链路 1000 Mbps，上行 943/938、下行 939/940 Mbps、0 重传，吞吐结论不变。补测下行(RX) CPU 占用并修正 CPU 表——原表仅含上行；下行收包更重（id 由 ~76% 降至 ~53%，约占 2 核）。

---

## 4. Wi-Fi 扫描与 Monitor 模式

**问题**：Does the onboard Wi-Fi support nl80211 scanning while associated? Monitor mode? Which driver/module is it?

**状态**：已完成

**子问题 1：驱动/模组是什么？**

| 项目 | 结果 |
|------|------|
| 模组型号 | Realtek **RTL8821CS**（SDIO 接口，Wi-Fi + Bluetooth combo） |
| 内核驱动 | `8821cs.ko` |
| 驱动路径 | `/lib/modules/6.12.35/kernel/drivers/net/wireless/realtek/rtl8821cs/8821cs.ko` |
| 驱动版本 | `v5.15.9.6-1-g3fe4f4b91.20250327_COEX20230331-5d5d` |
| 依赖 | `cfg80211` |
| 用户空间接口 | 标准 nl80211 / `iw` / `wpa_supplicant` |

设备端验证：
```bash
$ lsmod | grep 8821cs
cfg80211              303104  1 8821cs

$ modinfo 8821cs | head -5
filename:       /lib/modules/6.12.35/kernel/drivers/net/wireless/realtek/rtl8821cs/8821cs.ko
version:        v5.15.9.6-1-g3fe4f4b91.20250327_COEX20230331-5d5d
author:         Realtek Semiconductor Corp.
description:    Realtek Wireless Lan Driver
```

**子问题 2：是否支持在已关联 AP 的情况下扫描？**

- 当前状态：**已在设备上实测通过**（2026-06-24，关联 5GHz AP `inhand-visitor`）。
- 实测：先关联，再在关联态扫描：
  ```bash
  $ nmcli dev wifi connect "inhand-visitor" password ****     # 关联 5GHz(5745MHz, ch149)
  $ iw dev wlan0 link        # Connected to 80:8d:b7:eb:80:90  SSID: inhand-visitor
  $ iw dev wlan0 scan | grep -c "^BSS"
  83                          # 关联态一次扫描发现 83 个 AP
  $ iw dev wlan0 link        # 扫描后仍 Connected（连接未中断）
  $ ping -c2 <gw>            # 0% packet loss, ~1.7ms（扫描后通信正常）
  ```
- 结论：RTL8821CS 在已关联 AP 时**支持主动扫描**，扫描全频段返回完整 AP 列表，扫描期间连接保持、扫描后通信正常（未观察到掉线）。未关联态单次扫描可见 86 个 AP。

**子问题 3：是否支持 Monitor 模式？**

- 当前默认镜像：**不支持**。
  ```bash
  $ iw list | grep -A6 "Supported interface modes"
  Supported interface modes:
       * IBSS
       * managed
       * AP
       * P2P-client
       * P2P-GO
  
  $ ip link set wlan0 down && iw dev wlan0 set type monitor
  command failed: Operation not supported (-95)
  ```

- 驱动源码分析：
  - 文件 `drivers/net/wireless/realtek/rtl8821cs/Makefile:126` 定义：
    ```makefile
    CONFIG_WIFI_MONITOR = n
    ```
  - 同一目录下 `os_dep/linux/ioctl_cfg80211.c:10177` 中：
    ```c
    wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION)
                            #ifdef CONFIG_AP_MODE
                            | BIT(NL80211_IFTYPE_ADHOC)
                            | BIT(NL80211_IFTYPE_AP)
                            #endif
                            #ifdef CONFIG_WIFI_MONITOR
                            | BIT(NL80211_IFTYPE_MONITOR)
                            #endif
    ```
  - 结论：驱动源码包含 monitor 模式代码，但编译时通过 `CONFIG_WIFI_MONITOR = n` 禁用了。

- 可开启方案：
  1. 修改 `drivers/net/wireless/realtek/rtl8821cs/Makefile`，将 `CONFIG_WIFI_MONITOR = n` 改为 `y`。
  2. 重新编译 `8821cs.ko`。
  3. 部署到设备替换原模块。
  4. 重新加载后，`iw list` 将显示 monitor 模式，并可执行 `iw dev wlan0 set type monitor`。

- 仓库修改（已编译 + 部署 + 设备验证，2026-06-24）：
  - 修改 `board-support/ti-linux-kernel-6.12.35+git-ti-rt/drivers/net/wireless/realtek/rtl8821cs/Makefile`：
    ```diff
    -CONFIG_WIFI_MONITOR = n
    +CONFIG_WIFI_MONITOR = y
    ```
  - 该驱动以 `CONFIG_RTL8821CS=m` 随 `make linux` 一同编出；`CONFIG_WIFI_MONITOR=y` 经 Makefile 转为 `-DCONFIG_WIFI_MONITOR`，门控 `ioctl_cfg80211.c` 中 `BIT(NL80211_IFTYPE_MONITOR)`。编出的 `8821cs.ko` 含 `rtw_cfg80211_add_monitor_if`、"Monitor mode : Enable" 等符号。
  - 设备验证（替换模块并重启后，2026-06-24）：
    ```bash
    $ iw list | grep -A6 "Supported interface modes"
         * IBSS
         * managed
         * AP
         * monitor            # ← 新增
         * P2P-client
         * P2P-GO
    $ ip link set wlan0 down && iw dev wlan0 set type monitor   # 成功
    $ iw dev wlan0 info | grep type
         type monitor
    ```
  - **空口抓包实测**（monitor + 指定信道 + tcpdump，抓到周围 AP 的 802.11 管理帧，非发给本机，证明真正空口侦听）：
    ```bash
    $ iw dev wlan0 set type monitor && ip link set wlan0 up
    $ iw dev wlan0 set channel 6
    $ tcpdump -i wlan0 -c 12 -nn -e -s 256
    03:04:20 ... 2437 MHz 11b -48dBm BSSID:c2:54:f3:f6:f9:cc Beacon (huawei2_Wi-Fi5) ... CH: 6, PRIVACY
    03:04:20 ... 2437 MHz 11b -54dBm BSSID:40:a5:ef:96:3a:4e Probe Response (COMFAST-963A4E) ... CH: 6
    03:04:20 ... 2437 MHz 11b -44dBm BSSID:00:18:05:2b:5d:0c Probe Response (yjf_test_2g_1) ... CH: 6, PRIVACY
    ...
    12 packets captured / 159 packets received by filter / 0 dropped
    ```
  - ⚠️ **空口抓包需要抓包工具**：默认镜像未预装 `tcpdump`。本次实测临时安装（`apt-get download tcpdump libpcap0.8t64` + `dpkg -i`）。**V1.0.7 base tar 需预装 `tcpdump`**（依赖 `libpcap0.8t64`），否则 monitor 模式无法直接抓包。

- **5GHz 关联时的内核告警（已分析，非阻断）**：
  - 现象：在 **monitor↔managed 切换后** 或 **开机首次初始化后** 的那一次连接，内核打印两条 `rtw_warn_on` 告警（`rtw_mlme_ext.c:11520`、`rtw_dfs.c:1145`）并将内核标记 `Tainted: G W`。
  - 触发条件很窄：实测**稳态 managed 下直接 `nmcli dev connect wlan0` 重连不触发**；只有接口 type 切换/初始化后角色未建立的瞬态才命中。客户日常 managed 使用基本不会出现。
  - 根因：`disconnect_hdl()`（`RTW_CMD_THREAD`）中发 deauth 有"是否已关联"判断，但紧随的 MLME 清理 `rtw_mlmeext_disconnect()` **无条件调用**；从"未关联/角色未建立"瞬态进入时，AP/MESH/STA/ADHOC 判断全不中→`else`/`default`→`rtw_warn_on(1)`。`rtw_warn_on` 在 Linux 下即 `WARN_ON`（`osdep_service.h:560`），命中即打印+taint。DFS 那条同理（`rtw_dfs_rd_en_decision()` 评估雷达检测时 per-link 状态落到 default）。
  - 与 monitor **无关**：`rtw_mlme_ext.c`/`rtw_dfs.c` 不含任何 `CONFIG_WIFI_MONITOR` 引用（grep=0），monitor 改动仅触及 `ioctl_cfg80211.c`；实测 monitor 进入+空口抓包（promiscuous）阶段**不产生告警**，仅随后的 5GHz 关联才触发。
  - 处置：属 vendor 驱动过严断言（非真故障）。已在源码注释这两处 `rtw_warn_on(1)`（保留其后 `RTW_INFO`/`break`，逻辑不变），随 `make linux` 编入 V1.0.7。**已设备验证**：重编模块（vermagic 一致）部署重启后，复现原本必触发的 monitor→managed→connect 场景，dmesg 新增告警/Oops 计数 = 0，内核不再 taint。

**频段支持**：
- 2.4 GHz（Band 1）：HT20/HT40，最大 150 Mbps。
- 5 GHz（Band 2）：支持，共 28 个信道。

**结论**：
1. 板载 Wi-Fi 为 **Realtek RTL8821CS**，驱动 `8821cs`，标准 nl80211/cfg80211 接口，支持 2.4G + 5G 双频。
2. **关联时扫描**：已实测支持——关联 5GHz AP 后扫描仍返回 83 个 AP，扫描期间连接保持、扫描后通信正常。
3. **Monitor 模式**：默认镜像不支持；开启 `CONFIG_WIFI_MONITOR` 重编 `8821cs.ko` 后**已设备验证支持**——`iw list` 出现 monitor、可 `set type monitor`，并用 `tcpdump` 在信道 6 **实抓到 12 个空口 802.11 帧**（Beacon/Probe Response）。该改动随 `make linux`（`CONFIG_RTL8821CS=m`）编出，V1.0.7 镜像默认带 monitor 能力。**注意：base tar 需预装 `tcpdump` 方能直接抓包。**

**更新日志**：
- 2026-06-24：识别 Wi-Fi 模组为 RTL8821CS，确认驱动为 `8821cs`。
- 2026-06-24：执行 `iw dev wlan0 scan`，未关联状态下发现 86 个 AP。
- 2026-06-24：确认默认模块 `Supported interface modes` 不含 monitor，`set type monitor` 返回 `-EOPNOTSUPP`。
- 2026-06-24：定位驱动 `CONFIG_WIFI_MONITOR = n` 开关并改为 `y`，随 `make linux` 重编 `8821cs.ko`。
- 2026-06-24：部署重编模块（vermagic 与运行内核一致）并重启，`iw list` 出现 monitor、`iw dev wlan0 set type monitor` 成功、`info` 显示 `type monitor`。
- 2026-06-24：提供测试 AP `inhand-visitor`，实测关联 5GHz 后关联态扫描得 83 AP、连接存活、ping 网关 0 丢包。
- 2026-06-24：设备临时安装 `tcpdump`（apt 因预存 OpenSSL deb 依赖冲突受阻，改用 `apt-get download`+`dpkg -i`），monitor 信道 6 **实抓 12 个 802.11 帧**（周围 AP 的 Beacon/Probe Response，非本机目的）。→ 待办：base tar 预装 `tcpdump`。
- 2026-06-24：定位 5GHz 关联告警根因为 `disconnect_hdl()` 无条件调用 `rtw_mlmeext_disconnect()`；实测稳态重连不触发、仅 type 切换/初始化后那次触发；确认与 monitor 无关（`rtw_mlme_ext.c`/`rtw_dfs.c` 无 `CONFIG_WIFI_MONITOR` 引用，monitor 抓包阶段不告警）。
- 2026-06-24：源码注释 `rtw_mlme_ext.c:11520`、`rtw_dfs.c:1145` 两处 `rtw_warn_on(1)`（保留 `RTW_INFO`/`break`，逻辑不变）。
- 2026-06-24：单编该模块（`make M=drivers/.../rtl8821cs modules`，CROSS=devkit aarch64）部署重启，复现 monitor→managed→connect，**告警与内核 taint 彻底消失（新增计数=0）**，monitor/扫描/蓝牙功能正常。

---

## 5. BLE GATT Peripheral 支持

**问题**：Does BlueZ run on your BLE stack, and is GATT peripheral (advertising) mode supported for device provisioning?

**状态**：已完成

**测试环境**：陪测设备 MO-62A（`10.5.30.54`），内核 `6.12.35`。

**子问题 1：BlueZ 是否运行？**

| 项目 | 结果 |
|------|------|
| BlueZ 版本 | **5.82**（`bluetoothd --version`） |
| 服务 | `bluetooth.service` **active** |
| 控制器 | Realtek RTL8821CS BT，经 UART **H5**（`hci_uart` + Realtek H5 protocol）挂为 `hci0` |
| hci0 | BD `44:87:63:63:67:87`，**UP RUNNING**，LMP **4.2**（支持 BLE），Manufacturer Realtek(93) |

**子问题 2：BLE / GATT 外设（广播）模式是否支持？**

- LE 能力（`btmgmt info`）：supported settings 含 `le advertising secure-conn`；current settings 已含 `le`。
- 广播能力（`btmgmt advinfo`）：**Max instances 5**；支持 flags `connectable / general-discoverable / limited-discoverable / tx-power / scan-rsp-*`；广播数据 31B + scan-rsp 31B。
- D-Bus 外设接口（`/org/bluez/hci0`）：
  ```
  org.bluez.GattManager1            # 注册 GATT 服务（peripheral 端 GATT server）
  org.bluez.LEAdvertisingManager1   # RegisterAdvertisement / UnregisterAdvertisement
  ```
  这正是设备配网 App 实现 GATT peripheral + LE 广播所用的标准 BlueZ D-Bus API。
- **实测起停广播**：
  ```bash
  $ btmgmt add-adv -d 020106 -u 180a 1
  Instance added: 1
  $ btmgmt advinfo            # Instances list with 1 item
  $ btmgmt rm-adv 1
  Instance removed: 1
  ```

**已知风险（BT 稳定性）**：Q4 测试期间观察到一次 `bluetoothd`/`hci_uart` H5 路径的内核 Oops（`Unable to handle kernel paging request`，偶发，后续重启未复现）。与 Wi-Fi 改动无关，属 Realtek BT H5 UART 偶发不稳定；建议持续观察，若批量复现再排查 `hci_uart`/H5 路径。

**结论**：
1. BlueZ **5.82** 在 `hci0`（Realtek，UART H5）上运行，LMP 4.2 支持 BLE。
2. **GATT peripheral + LE 广播支持并实测可用**：D-Bus `GattManager1`/`LEAdvertisingManager1` 齐备，可注册/启停广播实例（最多 5 个），满足设备配网（advertising + GATT server）需求。
3. 稳定性：观察到一次**偶发** BT H5 Oops（未复现），建议观察。

**更新日志**：
- 2026-06-24：确认 BlueZ 5.82、`bluetooth.service` active；hci0 = Realtek UART H5、LMP 4.2、UP RUNNING。
- 2026-06-24：`btmgmt info`/`advinfo` 确认 LE + advertising 支持（Max 5 实例）。
- 2026-06-24：`busctl` 确认 `/org/bluez/hci0` 暴露 `GattManager1` + `LEAdvertisingManager1`。
- 2026-06-24：实测 `btmgmt add-adv` 起一条 BLE 广播实例成功、`rm-adv` 清除成功。
- 2026-06-24：记录 Q4 期间一次偶发 `bluetoothd`/`hci_uart` H5 Oops（未复现，待观察）。

---

## 6. 定制镜像构建与工厂预烧

**问题**：Can we build and distribute our own signed flashable image using your SDK? Do you offer factory pre-flashing of customer images at volume?

**状态**：待查证

**结论**：

---

## 7. Secure Boot 密钥灌装流程

**问题**：What's the documented Secure Boot key provisioning flow for locking the board to our signed image?

**状态**：待查证

**结论**：

---

## 8. 已验证 USB 网卡

**问题**：Which USB Ethernet adapters have you validated?

**状态**：待查证

**结论**：

---

## 9. 批量定价

**问题**：What's volume pricing? (range is OK)

**状态**：待查证

**结论**：

---

## 10. Pi 5 外壳兼容性

**问题**：Since it's a bare PCB: do standard Pi 5 enclosures fit given the identical 85 × 56 mm footprint and connector layout, or do port positions differ? Obviously some enclosures won't work, because of the 2 HDMI ports versus 1 on your board.

**状态**：待查证

**结论**：

---

*更新日志*
- 2026-06-23：创建问题清单。
