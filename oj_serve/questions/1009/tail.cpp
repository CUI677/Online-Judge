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

bool SameTree(TreeNode* a, TreeNode* b)
{
    if(a == nullptr && b == nullptr)
    {
        return true;
    }
    if(a == nullptr || b == nullptr)
    {
        return false;
    }
    if(a->val != b->val)
    {
        return false;
    }
    return SameTree(a->left, b->left) && SameTree(a->right, b->right);
}

TreeNode* BuildTree1()
{
    TreeNode* root = new TreeNode(4);
    root->left = new TreeNode(2);
    root->right = new TreeNode(7);
    root->left->left = new TreeNode(1);
    root->left->right = new TreeNode(3);
    root->right->left = new TreeNode(6);
    root->right->right = new TreeNode(9);
    return root;
}

TreeNode* BuildExpected1()
{
    TreeNode* root = new TreeNode(4);
    root->left = new TreeNode(7);
    root->right = new TreeNode(2);
    root->left->left = new TreeNode(9);
    root->left->right = new TreeNode(6);
    root->right->left = new TreeNode(3);
    root->right->right = new TreeNode(1);
    return root;
}

TreeNode* BuildTree2()
{
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    root->right = new TreeNode(3);
    return root;
}

TreeNode* BuildExpected2()
{
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(3);
    root->right = new TreeNode(1);
    return root;
}

TreeNode* BuildTree3()
{
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(2);
    root->left->left = new TreeNode(3);
    return root;
}

TreeNode* BuildExpected3()
{
    TreeNode* root = new TreeNode(1);
    root->right = new TreeNode(2);
    root->right->right = new TreeNode(3);
    return root;
}

void Test(TreeNode* root, TreeNode* expected, const char* test_name)
{
    TreeNode* actual = Solution().invertTree(root);
    if(SameTree(actual, expected))
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed" << endl;
    }
    DestroyTree(actual);
    DestroyTree(expected);
}

int main()
{
    Test(BuildTree1(), BuildExpected1(), "Test1");
    Test(BuildTree2(), BuildExpected2(), "Test2");
    Test(BuildTree3(), BuildExpected3(), "Test3");
    Test(nullptr, nullptr, "Test4");
    return 0;
}
