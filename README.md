# fuse-ext2
这是一个简化版的ext2文件系统，支持用户态环境下的挂载。


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