#include <iostream>
#include <vector>
using namespace std;

struct ListNode
{
    int val;
    ListNode* next;

    explicit ListNode(int x)
        : val(x)
        , next(nullptr)
    {
    }
};

class Solution
{
public:
    bool hasCycle(ListNode* head)
    {
