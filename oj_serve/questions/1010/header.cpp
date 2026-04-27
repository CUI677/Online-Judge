#include <functional>
#include <iostream>
#include <limits>
using namespace std;

struct TreeNode
{
    int val;
    TreeNode* left;
    TreeNode* right;

    explicit TreeNode(int x)
        : val(x)
        , left(nullptr)
        , right(nullptr)
    {
    }
};

class Solution
{
public:
    bool isValidBST(TreeNode* root)
    {
