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
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);
    return root;
}

string FormatLevels(const vector<vector<int>>& levels)
{
    ostringstream out;
    out << "[";
    for(size_t i = 0; i < levels.size(); ++i)
    {
        if(i > 0)
        {
            out << ",";
        }
        out << "[";
        for(size_t j = 0; j < levels[i].size(); ++j)
        {
            if(j > 0)
            {
                out << ",";
            }
            out << levels[i][j];
        }
        out << "]";
    }
    out << "]";
    return out.str();
}

void Test(TreeNode* root, const vector<vector<int>>& expected, const char* test_name)
{
    vector<vector<int>> actual = Solution().levelOrder(root);
    if(actual == expected)
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed, expected: " << FormatLevels(expected)
             << ", actual: " << FormatLevels(actual) << endl;
    }
    DestroyTree(root);
}

int main()
{
    Test(BuildTree1(), vector<vector<int>>{{3}, {9, 20}, {15, 7}}, "Test1");
    Test(BuildTree2(), vector<vector<int>>{{1}, {2, 3}, {4, 5}}, "Test2");
    Test(nullptr, vector<vector<int>>{}, "Test3");
    return 0;
}
