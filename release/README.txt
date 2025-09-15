OpenArk v1.5.2
------------------------------------------------------------------
工具库全新升级，支持在线仓库更新、用户自定义工具等功能
句柄管理支持将FILE_HANDLE保存到文件
BUG修复、稳定性增强等未提及的功能
--------------------------------------------------------------------
ToolRepo fully upgraded, supporting online repository updates, user-defined tools, and other features
Handle management now supports saving FILE_HANDLE to a file
Bug fixes, stability improvements, and other enhancements not explicitly mentioned
------------------------------------------------------------------

OpenArk v1.5.0
--------------------------------------------------------------------
- 进程管理增加PID暴力搜索，列表清除、服务定位等功能
- 内存搜索支持显示模块区域，内存编辑支持更多语法
- 优化内核模式以及ELF扫描功能支持文件编辑等
- BUG修复、稳定性增强以及许多未提及的功能
------------------------------------------------------------------
- Process management now supports PID brute-force search, list clearing, and service location features.
- Memory search can now display module regions; memory editing supports more syntax.
- Kernel-mode and ELF scanning optimized, now supporting file editing and more.
- Bug fixes, stability improvements, and many other enhancements not listed here.
--------------------------------------------------------------------

OpenArk v1.3.8
--------------------------------------------------------------------
- 支持隐身模式，增加Beta通道
- 内核增强：新增枚举ImageVerification/Bounds/KernelHash回调以及EPROCESS信息等
- 界面优化：新增保存过滤历史记录，包含进程树等
- 热键功能修复，工具库新增工具
- BUG修复以及一些未提及的功能
- 新年快乐 2025 🐍
------------------------------------------------------------------
- Support invisible mode and added beta channel.
- Impoved kernel manager: Added enum ImageVerification/Bounds/KernelHash callbacks and EPROCESS info etc.
- Improved UI: Added filter history and  contain process tree etc.
- Enum hotkey fixed  and added some tools.
- Bugfixed and added some features not mentioned .
- Happy Chinese New Year 2025 🐍
--------------------------------------------------------------------

OpenArk v1.3.6
--------------------------------------------------------------------
- 进内核模式支持离线环境(无网络情况)
- 内核增强：支持NPFS/MailSlot/MUP过滤驱动枚举等
- 界面优化：新增繁体中文语言、启动项支持多选删除等
- BUG修复，新增一些工具
------------------------------------------------------------------
- Enter kernel mode works in an offline environment(network unavailable).
- Impoved kernel manager: Added NPFS/MailSlot/MUP list etc.
- Improved UI: Added zh-tw language, and can be deleted in batches.
- Bugfixed and added some tools.
--------------------------------------------------------------------

OpenArk v1.3.4
--------------------------------------------------------------------
- 进程增强：增加内存使用、PEB、TEB、线程栈、结束线程等各种功能
- 内核增强：增加全内存搜索、卸载驱动列表、镜像劫持、加载符号等各种功能
- 扫描提升：优化PE扫描、支持解析内存化PE等功能
- 解决部分不能进入内核模式的问题
- BUG修复，还有其它很多未提及的功能
- 特别说明：增加致谢名单，感谢对OpenArk的支持！
- Impoved process manager: Added memory usage, PEB, TEB, CallStack, Terminate Thread etc.
- Impoved kernel manager: Added memory search, Unloaded drivers, IFEO, Load symbols etc.
- Improved scanner: Improved pe scanner, Added scanner for Memory PE.
- Fixed some failure case when enter kernel mode.
- Bugfixed and many other unmentioned features.
- Special Notes: Added acknowledgements, thanks for your support!
--------------------------------------------------------------------

OpenArk v1.3.2 
--------------------------------------------------------------------
进程增强：增加PPL、内存扫描、线程管理、模块卸载、句柄提权等各种功能
内核增强：增加禁用/启用回调，Dump驱动、消息钩子、强删文件、文件/注册表管理、启动项、计划任务、服务管理等各种功能
界面增强：优化UI，大幅提升流畅性
支持最新Win11
BUG修复，还有其它很多未提及的功能
Keep Simple, Keep Evolutionary!
Impoved process manager: Added PPL,MemoryScan,Thread,unload module、change handle access etc.
Impoved kernel manager: Added kernel features, enable/disable callback, Dump driver,MessageHook,ForceDelete,File/Reg/Boot manager etc.
Improved UI substantially.
Support win11 latest release.
Bugfixed and many other unmentioned features.
--------------------------------------------------------------------

OpenArk v1.3.0 
--------------------------------------------------------------------
进程管理增强，自动刷新，支持拖动选择窗口
内核增加SSDT/Shadow/WFP/Filter/Minifilter/Timer等多种回调
热键支持Win11
工具库合并，更新换代，支持查询
扫描器增加ELF、普通文件扫描
增加数学计算等编程助手
各种BUG修复，各种记不清的功能
时隔一年，只能说，Good Luck 2023！
Impoved process manager, refresh automatically.
Added kernel features, SSDT/Shadow/WFP/Filter/Minifilter/Timer etc.
Added enum hotkey for win11.
Added tools repo, search supported.
Added ELF/FILE scanner .
Added match caculator in codekits.
Bugfixed and other features.
Good Luck 2023!
--------------------------------------------------------------------

OpenArk v1.2.2 
--------------------------------------------------------------------
增加调试符号设置、开发了国内加速镜像节点
程序支持在线升级
工具目录支持修改、增加一些工具
支持数据导出功能，例如热键、驱动列表等
查看进程模块时可过滤系统模块
PE扫描器支持下载微软pdb符号文件
支持拖拽文件查看文件占用
新年快乐 2022！
Add debug system and cn-mirror repo features.
Add online upgrade feature.
Add change reverse tool folder and some tools.
Add feature that export data to file.
Optimized module, support hide system modules etc.
PE scanner support download microsoft pdb file
Support drag file to view unlock file
Happy Chinese New Year 2022!
--------------------------------------------------------------------

OpenArk v1.2.0
--------------------------------------------------------------------
支持win11 21H2
汇编工具支持arm/mips 16/32/64 大小端
增加更多工具加速逆向分析
修复进入内核模式失败问题 #57 #25 #52 #58 #53 #50 
部分界面优化
修复BUG #60 
--------------------------------------------------------------------
+ support win11 21H2
+ asmtools support arm/mips 16/32/64 be/le
+ add more tools for easy reversing.
+ fix enter kernel mode failed. issue #57 #25 #52 #58 #53 #50 
+ ui optmized
+ fix bug #60 
--------------------------------------------------------------------

OpenArk v1.1.0
--------------------------------------------------------------------
支持Win10 20H2
支持配置文件和工具可移植，issue #23
代码和界面优化
增加URL编解码功能
修复高分辨率显示异常, issue #24
修复无法获取其它会话的内存区bug
修复其它bug, 还有一些没提到的功能点
--------------------------------------------------------------------
support win10 20H2
support portable config and tools, issue #23
code and ui optmized
add url encode/decode features
fix DPI view error bug. issue #24
fix can't get other sessions bug in memory section view
fix others bugs, some changes no longer mention...
--------------------------------------------------------------------

OpenArk v1.0.8
--------------------------------------------------------------------
增加内核存储模块，目前支持查看占用，文件解锁等功能（Unlocker）
增加内存编辑器功能，支持内核和用户态
增加内核对象模块，支持读写内存共享MAP，查看类型对象
增加网络管理模块，支持查看端口进程、编辑hosts文件等
增加大量方便的逆向开发人员的工具
增强文字编码功能，支持分割符
代码和界面优化
修复部分bug, 还有一些没提到的功能点
--------------------------------------------------------------------
add kernel storage, aka file unlocker
add kernel memory editor, r0 and r3 supported
add kernel object, support read and write shared named map, object types view.
add kernel network as port and hosts file manager etc.
add more usefull tools for coder/reverse engineers
enhanced codec features, support separator
code and ui optmized
fix some bugs, some changes no longer mention...
--------------------------------------------------------------------

OpenArk Daliy-Build（日常构建）
--------------------------------------------------------------------
内核：增加枚举和摘除系统热键的功能，支持Win7/Win8/Win8.1/Win10 (~2004) x86/x64 所有版本)
--------------------------------------------------------------------
kernel: add enum/remove system hotkeys features，support Win7/Win8/Win8.1/Win10 (~2004) x86/x64 all versions)
--------------------------------------------------------------------

OpenArk v1.0.6
--------------------------------------------------------------------
增加逆向工具，持续更新
系统工具支持快速关机重启、结束资源管理器等功能
优化垃圾清理功能
菜单栏增加运行功能
支持记录上一次打开的标签页、窗口大小等
代码和界面优化
修复部分bug
--------------------------------------------------------------------
add reverse tools，update on going.
add fast reboot/poweroff, kill/reset explorer and so on.
enhanced cleaner features.
add run features in menu.
record the last opened tab and window size
code and ui optmized
fix some bugs
--------------------------------------------------------------------

OpenArk v1.0.4
--------------------------------------------------------------------
内核（XP、（Win7、8、10 x64）等版本已测试）：
新增签名、无签名驱动安装
新增内核基本信息显示
新增LoadImage、CreateProcess、CreateThread、CmpCallback回调查看、反汇编以及删除等。
新增内核内存Dump功能（读取、反汇编）
支持进程树切换时当前选定进程不丢失
新增进程列表右键重启
修复部分bug
--------------------------------------------------------------------
add kernel notifies view，memory view，install not signed driver, and more...
--------------------------------------------------------------------

OpenArk v1.0.2
--------------------------------------------------------------------
支持多语言，目前支持中英文切换
进程树模式支持
光标移到进程名上，显示命令行和路径
模块栏增加查看属性
新增进程句柄、内存列表操作
UI布局修改
签名、版本信息查看
签名信息、Hash值计算
新增.ps -rst（重启进程）
新增.pstree（查看进程树）
命令历史记录自动保存到本地等等..
新增命令历史记录路径和最大条数配置等等..
新增Base64、CRC32、MD5、SHA1计算
新增查看驱动列表（扫描、定位、查看属性等）
新增驱动安装功能
--------------------------------------------------------------------
add support ProcessTree、Memory、Handles、Cryptor、KernelDriver view and install，enahanced UI more and more...
--------------------------------------------------------------------

OpenArk v1.0.0
--------------------------------------------------------------------
OpenArk 第一版问世
--------------------------------------------------------------------
The first version of OpenArk is online
--------------------------------------------------------------------