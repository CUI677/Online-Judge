#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

    }
};

void Test(const string& name, const string& expected, const char* test_name)
{
    string actual = Solution().HelloName(name);
    if(actual == expected)
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed, expected: " << expected
             << ", actual: " << actual << endl;
    }
}

int main()
{
    Test("Alice", "hello Alice", "Test1");
    Test("Bob", "hello Bob", "Test2");
    Test("", "hello ", "Test3");
    return 0;
}
