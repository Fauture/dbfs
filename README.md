# dbfs文件解包 asktao

该程序只可获取文件内数据,添加数据请自行编写

注意,解压的文件需要lzss解压 文件前20字节分别是下面的数据
</br>int 压缩类型 1 未压缩 5 lzss压缩
</br>int 时间校验
</br>int 时间校验
</br>int 解压后长度
</br>int 解压前长度
