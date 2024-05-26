# 一个快捷键启动工具

## 工具
* Qt: 6.4.3
* vs: 2022
* c++ >= 17

## 功能
* 可将任意文件或任意文件夹通过绑定的快捷键快速打开
* 文件夹和文件通过默认应用打开，exe直接打开
* 可将左右Ctrl和Alt区分，小键盘数字与主键盘数字区分，实现超多组合
* 支持配置触发次数：单次，两次，三次 触发 == (多次触发不包括 ctrl 和 alt ，比如设置的是 ctrl+n ,那么就是按住ctl然后按下 两/三 次 n 键) ==
* 可 创建/切换 配置文件，增加/删除，启用/停用单行快捷键配置
* 无需通过注册表即可设置开机启动
* 简单易用的日志消息
* 配置简单，全可视化
* 支持 中/英 语言轻松切换

## 使用
### 解压点击exe即可
### 如需自己编译，注意默认的是英语。中文找到对应的qm文件，然后在生成的exe同级目录下新建 translations 文件夹，将qm文件放入即可识别

#### 注意:  本程序执行路径下不要有中文(不影响程序内设置绑定快捷项目是否有中文)(qt的好像都这样,希望有好心人告诉我怎么解决)
#### 注意:  输入文件框有三种方式， 拖拽,双击,直接输入。 其中双击无法选择文件夹
#### 注意:  请别用过于妖孽的组合键，如alt+f4这种系统自带的，虽然可以用，但也会唤醒系统原有的功能
#### 注意:  不要狂搓键盘中的时候突然按下设置的快捷键，此种情况不一定触发(狂搓是指别一秒十几个键那种)，如未触发，稍等0.5秒左右尝试

## 演示
### 1.配置的创建与切换
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/1_%E9%85%8D%E7%BD%AE%E7%9A%84%E5%88%9B%E5%BB%BA%E4%B8%8E%E5%88%87%E6%8D%A2.gif?raw=true)

### 2.创建一行配置
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/2._%E5%88%9B%E5%BB%BA%E4%B8%80%E8%A1%8C%E9%85%8D%E7%BD%AE.gif?raw=true)
 
### 3.简单测试快捷键
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/3_%E7%AE%80%E5%8D%95%E6%B5%8B%E8%AF%95%E5%BF%AB%E6%8D%B7%E9%94%AE.gif?raw=true)
 
### 4.ctrl与alt组合演示
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/4_ctrl%E4%B8%8Ealt%E7%BB%84%E5%90%88%E6%BC%94%E7%A4%BA.gif?raw=true)

### 5.删除一行配置演示
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/5_%E5%88%A0%E9%99%A4%E4%B8%80%E8%A1%8C%E9%85%8D%E7%BD%AE%E6%BC%94%E7%A4%BA.gif?raw=true)

### 6.开机启动演示
![1_配置的创建与切换](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/6_%E5%BC%80%E6%9C%BA%E5%90%AF%E5%8A%A8%E6%BC%94%E7%A4%BA.gif?raw=true)

## 下载地址

## 项目结构 
![项目结构](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_key/%E7%BB%93%E6%9E%84.png?raw=true)
