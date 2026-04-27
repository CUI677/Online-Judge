#ifndef COMPILER_ONLINE
#include "header.cpp"
#endif

    }
};

ListNode* BuildList(const vector<int>& values)
{
    ListNode dummy(0);
    ListNode* tail = &dummy;
    for(int value : values)
    {
        tail->next = new ListNode(value);
        tail = tail->next;
    }
    return dummy.next;
}

vector<int> ToVector(ListNode* head, size_t limit = 64)
{
    vector<int> result;
    unordered_set<ListNode*> visited;
    while(head != nullptr && visited.insert(head).second && result.size() < limit)
    {
        result.push_back(head->val);
        head = head->next;
    }
    if(head != nullptr)
    {
        result.push_back(2147483647);
    }
    return result;
}

void DestroyList(ListNode* head)
{
    unordered_set<ListNode*> visited;
    while(head != nullptr && visited.insert(head).second)
    {
        ListNode* next = head->next;
        delete head;
        head = next;
    }
}

string FormatVector(const vector<int>& values)
{
    ostringstream out;
    out << "[";
    for(size_t i = 0; i < values.size(); ++i)
    {
        if(i > 0)
        {
            out << ",";
        }
        if(values[i] == 2147483647)
        {
            out << "...";
        }
        else
        {
            out << values[i];
        }
    }
    out << "]";
    return out.str();
}

void Test(const vector<int>& a, const vector<int>& b, const vector<int>& expected, const char* test_name)
{
    ListNode* list1 = BuildList(a);
    ListNode* list2 = BuildList(b);
    ListNode* actual_head = Solution().mergeTwoLists(list1, list2);
    vector<int> actual = ToVector(actual_head);

    if(actual == expected)
    {
        cout << test_name << " ... OK" << endl;
    }
    else
    {
        cout << test_name << " ... Failed, expected: " << FormatVector(expected)
             << ", actual: " << FormatVector(actual) << endl;
    }

    DestroyList(actual_head);
}

int main()
{
    Test(vector<int>{1, 2, 4}, vector<int>{1, 3, 4}, vector<int>{1, 1, 2, 3, 4, 4}, "Test1");
    Test(vector<int>{}, vector<int>{}, vector<int>{}, "Test2");
    Test(vector<int>{}, vector<int>{0}, vector<int>{0}, "Test3");
    return 0;
}
