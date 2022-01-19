test_cploxplox中的测试用例来源自https://github.com/munificent/craftinginterpreters/tree/master/test

由于cploxplox和原始的lox有一些语法出入，因此我对所有用例做了针对性修改，包括：
1. 注释由//改为#
2. 函数声明由fun改为func
3. print从语法变为函数
4. 继承由<改为>

拷贝或自行编译一个cploxplox.exe，放置于此文件夹中，通过powershell运行Tester.ps1脚本即可开始测试