#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

    }
};

void Test(int a, int b, int expected, const char* test_name)
{
    int actual = Solution().Add(a, b);
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
    Test(1, 2, 3, "Test1");
    Test(-5, 3, -2, "Test2");
    Test(100, 200, 300, "Test3");
    return 0;
}
