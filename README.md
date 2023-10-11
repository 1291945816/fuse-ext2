# fuse-ext2
这是一个简化版的ext2文件系统，支持用户态环境下的挂载。


### 如何去构建项目？
```shell
git clone git@github.com:1291945816/fuse-ext2.git
cd fuse-ext2
cmake -S . -Bbuild
cmake --build build --target fuse-ext2 -j 8
```