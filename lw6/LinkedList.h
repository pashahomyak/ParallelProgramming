#pragma once
#include<string>
#include<memory>

class LinkedList
{
	struct Node
	{
		Node(std::string inputData):
			data(inputData)
		{
		}

		std::string data;
		Node* next;
	};

public:
	LinkedList();
	~LinkedList();

	void Push(std::string data);
	void Pop();
	void Clear();
	size_t GetSize();
	std::string GetHeadData();

private:
	Node* head;
	std::size_t size;
};