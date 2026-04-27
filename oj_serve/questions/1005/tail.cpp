#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

    }
};

void DestroyTree(TreeNode* root)
{
    if(root == nullptr)
    {
        return;
    }
    DestroyTree(root->left);
    DestroyTree(root->right);
    delete root;
}

TreeNode* BuildTree1()
{
    TreeNode* root = new TreeNode(3);
    root->left = new TreeNode(9);
    root->right = new TreeNode(20);
    root->right->left = new TreeNode(15);
    root->right->right = new TreeNode(7);
    return root;
}

TreeNode* BuildTree2()
{
    TreeNode* root = new TreeNode(1);
    root->right = new TreeNode(2);
    return root;
}

void Test(TreeNode* root, int expected, const char* test_name)
{
    int actual = Solution().maxDepth(root);
    if(actual == expected)
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed, expected: " << expected
             << ", actual: " << actual << endl;
    }
    DestroyTree(root);
}

int main()
{
    Test(BuildTree1(), 3, "Test1");
    Test(BuildTree2(), 2, "Test2");
    Test(nullptr, 0, "Test3");
    return 0;
}
