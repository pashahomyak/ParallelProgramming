#include "LinkedList.h"

LinkedList::LinkedList():
	size(0),
	head(nullptr)
{
}

LinkedList::~LinkedList()
{
	Clear();
}

void LinkedList::Push(std::string data)
{
	if (auto node = new Node(data))
	{
		node->next = head;
		head = node;
		++size;
	}
}

void LinkedList::Pop()
{
	if (head)
	{
		auto newHead = head->next;
		delete head;
		head = newHead;
		--size;
	}
}

void LinkedList::Clear()
{
	while (head)
	{
		Pop();
	}
}

size_t LinkedList::GetSize()
{
	return size_t(size);
}

std::string LinkedList::GetHeadData()
{
	return head->data;
}