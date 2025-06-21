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
