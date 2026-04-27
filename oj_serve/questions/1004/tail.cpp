#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

    }
};

ListNode* BuildList(const vector<int>& values, vector<ListNode*>* nodes)
{
    ListNode dummy(0);
    ListNode* tail = &dummy;
    for(int value : values)
    {
        ListNode* node = new ListNode(value);
        nodes->push_back(node);
        tail->next = node;
        tail = node;
    }
    return dummy.next;
}

void DestroyNodes(const vector<ListNode*>& nodes)
{
    for(ListNode* node : nodes)
    {
        delete node;
    }
}

void Test(const vector<int>& values, int pos, bool expected, const char* test_name)
{
    vector<ListNode*> nodes;
    ListNode* head = BuildList(values, &nodes);
    if(pos >= 0 && pos < static_cast<int>(nodes.size()))
    {
        nodes.back()->next = nodes[pos];
    }

    bool actual = Solution().hasCycle(head);

    if(pos >= 0 && pos < static_cast<int>(nodes.size()))
    {
        nodes.back()->next = nullptr;
    }

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

    DestroyNodes(nodes);
}

int main()
{
    Test(vector<int>{3, 2, 0, -4}, 1, true, "Test1");
    Test(vector<int>{1, 2}, 0, true, "Test2");
    Test(vector<int>{1}, -1, false, "Test3");
    return 0;
}
