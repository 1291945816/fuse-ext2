# fuse-ext2
> 不适宜在多线程环境下使用
这是一个简化版的ext2文件系统，支持用户态环境下的挂载。

### 环境要求
1. C11标准


### 如何去构建项目？
```shell
git clone git@github.com:1291945816/fuse-ext2.git
cd fuse-ext2
cmake -S . -Bbuild
cmake --build build --target fuse-ext2 -j 8
```

### 运行须知
需要提供一个模拟该文件系统的文件，目前仅测试过`1GB`文件的大小,下面命令会在指定位置进行生成：
```shell
cd fuse-ext2
rm -rf image
mkdir image && cd image
touch fext2
# 生成一个1GB大小的文件 块大小为1KB
dd if=/dev/zero of=fext2 bs=1024 count=1048576
```

### 挂载文件系统
> 编译后的文件系统名称为 fuse-ext2
- `${mount_point}`：挂载点 
- `${mount_image}`：指定要求生成的镜像文件

```shell
./fuse-ext2 ${mount_point} --mount_image ${mount_image} 
```

**一个例子**
> 假设 在所挂载的文件系统路径下 存在一个模拟该文件系统所管理的磁盘文件
```shell
mkdir test 
./fuse-ext2 test --mount_image fext2
# 如果想在挂载后能够被其他用户所访问，可以通过增加 allow_user ,同时需要配置 /etc/fuse.conf 里面的内容 （去掉user_allow_other的注释）
./fuse-ext2  -o allow_other test --mount_image fext2 
```

**查看帮助信息**
```shell
./fuse-ext2 -h
```
