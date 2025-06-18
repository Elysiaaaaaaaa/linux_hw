# 《Linux 内核分析》课程项目
使用Linux高级IPC机制实现一个多进程系统来模拟一个双向隧道内的交通流量控制系统,详细表述见说明pdf

## 代码结构
```
.
├── CMakeLists.txt
├── Makefile
├── README.md
├── build               
├── include             头文件
├── input               输入样例
├── linux_hw            可执行文件
└── src                 cpp文件
```

## input formate
首先5行确定超参数

```angular2html
total_number_of_cars  
maximum_number_of_cars_in_tunnel 
tunnel_travel_time 
total_number_of_mailboxes 
memory_segment_size
```
- 通过的车数
- 隧道最多同时容纳的车数
- 车辆通过隧道的基准时间（真实值波动在30%）
- 隧道内可访问的信箱数
- 信箱大小


随后是每辆车的具体描述


每辆车均以car开始，end结束

```angular2html
car car_id direction
operation duration mailbox-no
...
end
```

car_id direction分别指定车的标识符和方向（0/1）

操作有w,r两种
```angular2html
w '写入内容' 写入时间 目标邮箱
r  读取长度  读取时间 目标邮箱
```
## 调度参数调整
可见`include/process.h`中`process`类的实现
```
//    调度算法选择，是否使用红绿灯
bool use_rg = true;
//    输入时间格式改变
bool ex_input = true;
```


## RUN
1. `build` 目录下执行以下指令编译
```
cmake ..
make -j32
```
2. 转移到项目目录
```angular2html
cd ..
```
3. 随机选取一个input运行
```angular2html
./linux_hw ./input/test.txt
./linux_hw ./input/test2.txt
./linux_hw ./input/test3.txt
```


## TODO
- [x] 类别构建
- [x] 同步组件
- [x] 共享内存实现
- [x] 毫秒级时间实现
- [x] 输出规格统一 car.show
- [x] 输出规格统一 process::leave>>car->model_str
- [x] readMailbox debug
- [x] 时间转换
- [ ] 测试数据构造
- [ ] 演示视频
- [ ] 报告
- [ ] 磕头
- [ ] 磕头
- [ ] 磕头
- [ ] 磕头