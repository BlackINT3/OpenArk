# OpenArk Code Style Guide (OpenArk 代码风格)

## File / Directory (文件/目录命名)
* File Name (Lower Case hyphen Joined or short lowercase) 小写和横线相连 或者 全小写简写，注意不要过长。
```
process-mgr.cpp
coderkit.cpp
process.c
```


## Function （函数命名）
* Function Name：Upper Camel Case (大驼峰命名)
* Function Param: The same as Variable (和变量命名一致)
```
QString CoderKit::NasmDisasm(const std::string &data, int bits)
{
    // ...
}
```

## Variable（变量命名）
* Global Variable (Lower Case Underline) 小写+下划线
* General Variable (Lower Case Underline) 小写+下划线，能描述清楚时，尽量简写
```
QApplication *app = nullptr;
std::wstring data;
std::string str;
```

## Class (类命名)
* Class Name: (Upper Camel Case) 类名大驼峰
* Member Name: (Underline end) 成员变量最后带下划线（Qt的ui的特殊变量除外）
* Qt Message Routine: (Lower Camel Case) Qt消息处理函数，使用小驼峰命名

```
class Settings : public QWidget {
	Q_OBJECT
public:
	Settings(QWidget *parent);
	~Settings();
protected:
	void closeEvent(QCloseEvent *e);
	void InitConsoleView();
	void InitCleanView();
	void InitGeneralView();

private:
	Ui::Settings ui;
	QStandardItemModel *console_model_;
};
```

## Flow Branch (流程分支)
* if/else (大括号不换行)
```
if (phd == 1) {
    return 1;
} else if (phd >= 2) {
    return 2
} else {
    return 3;
}
```

* switch (大括号不换行)
```
switch (type) {
case CREATE_PROCESS:
    ret= GetProcessNotifyInfo(count, items);
    break;
case CREATE_THREAD:
    ret = GetThreadNotifyInfo(count, items);
    break;
case LOAD_IMAGE:
    ret = GetImageNotifyInfo(count, items);
    break;
case CM_REGISTRY:
    ret = GetRegistryNotifyInfo(count, items);
    break;
default:
    break;
}
```

* for/while (大括号不换行)
```
for (int i = 0; i < notify->count; i++) {
    routines.push_back(notify->items[i]);
}

while (imp->Name != 0) {
    InitTableItem(imp_model_);
    imp++;
}

do {
    if (code) break;

    if (style) {
        printf("hello");
        break;
    }
} while (0);

```