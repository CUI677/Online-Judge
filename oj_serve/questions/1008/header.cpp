#include <iostream>
#include <sstream>
#include <unordered_set>
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
    void reorderList(ListNode* head)
    {
