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
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    root->right = new TreeNode(3);
    return root;
}

TreeNode* BuildTree2()
{
    TreeNode* root = new TreeNode(5);
    root->left = new TreeNode(1);
    root->right = new TreeNode(4);
    root->right->left = new TreeNode(3);
    root->right->right = new TreeNode(6);
    return root;
}

TreeNode* BuildTree3()
{
    TreeNode* root = new TreeNode(8);
    root->left = new TreeNode(3);
    root->right = new TreeNode(10);
    root->left->left = new TreeNode(1);
    root->left->right = new TreeNode(6);
    root->left->right->left = new TreeNode(4);
    root->left->right->right = new TreeNode(7);
    root->right->right = new TreeNode(14);
    root->right->right->left = new TreeNode(13);
    return root;
}

TreeNode* BuildTree4()
{
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    return root;
}

void Test(TreeNode* root, bool expected, const char* test_name)
{
    bool actual = Solution().isValidBST(root);
    if(actual == expected)
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed, expected: "
             << (expected ? "true" : "false")
             << ", actual: " << (actual ? "true" : "false") << endl;
    }
    DestroyTree(root);
}

int main()
{
    Test(BuildTree1(), true, "Test1");
    Test(BuildTree2(), false, "Test2");
    Test(BuildTree3(), true, "Test3");
    Test(BuildTree4(), false, "Test4");
    return 0;
}
