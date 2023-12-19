#include "pch.h"
#include "wmcv_freelist_best_fit_policy_detail.h"
#include "wmcv_allocator_utility.h"

namespace wmcv::detail
{
static constexpr size_t BLACK = 0;
static constexpr size_t RED   = 1;

static_assert(sizeof(Node) == 48);

void Insert(Node*& root, Node* to_insert) noexcept
{
	auto* curr = root;
	auto* prev = Node::SENTINEL();

	while ( curr != Node::SENTINEL() )
	{
		prev = curr;
		if (to_insert->size < curr->size)
		{
			curr = curr->children[LEFT];
		}
		else
		{
			curr = curr->children[RIGHT];
		}
	}

	to_insert->parent = prev;

	if (prev == Node::SENTINEL())
	{
		root = to_insert;
	}
	else if (to_insert->size < prev->size)
	{
		prev->children[LEFT] = to_insert;
	}
	else
	{
		prev->children[RIGHT] = to_insert;
	}

	to_insert->children[LEFT] = Node::SENTINEL();
	to_insert->children[RIGHT] = Node::SENTINEL();
	to_insert->color = RED;

	PostInsertRebalance(root, to_insert);
}

void Remove(Node*& root, Node* z) noexcept
{
	Node* x = Node::SENTINEL();
	Node* y = z;
	size_t y_original_color = y->color;

	if ( z->children[LEFT] == Node::SENTINEL() )
	{
		x = z->children[RIGHT];
		Transplant(root, z, z->children[RIGHT]);
	}
	else if ( z->children[RIGHT] == Node::SENTINEL() )
	{
		x = z->children[LEFT];
		Transplant(root, z, z->children[LEFT]);
	}
	else
	{
		y = Minimum(z->children[RIGHT]);
		y_original_color = y->color;
		x = y->children[RIGHT];

		if (y != z->children[RIGHT])
		{
			Transplant(root, y, y->children[RIGHT]);
			y->children[RIGHT] = z->children[RIGHT];
			y->children[RIGHT]->parent = y;
		}
		else
		{
			x->parent = y;
		}

		Transplant(root, z, y);
		y->children[LEFT] = z->children[LEFT];
		y->children[LEFT]->parent = y;
		y->color = z->color;
	}

	if (y_original_color == BLACK)
	{
		PostRemoveRebalance(root, x);
	}
}

auto IsRed(const Node* node) noexcept -> bool
{
	return node && node->color == RED;
}

auto IsBlack(const Node* node) noexcept -> bool
{
	return !IsRed(node);
}

template< size_t DIRECTION >
static void Rotate(Node*& root, Node* x) noexcept
{
	static_assert(DIRECTION == LEFT || DIRECTION == RIGHT);
	constexpr size_t DIR = DIRECTION;
	constexpr size_t OPP = size_t{1 - DIRECTION};

	Node* y = x->children[OPP];
	x->children[OPP] = y->children[DIR];

	if (y->children[DIR] != Node::SENTINEL())
	{
		y->children[DIR]->parent = x;
	}

	y->parent = x->parent;

	if ( x->parent == Node::SENTINEL() )
	{
		root = y;
	}
	else if (x == x->parent->children[DIR])
	{
		x->parent->children[DIR] = y;
	}
	else
	{
		x->parent->children[OPP] = y;
	}

	y->children[DIR] = x;
	x->parent = y;
}

void RotateLeft(Node*& root, Node* x) noexcept
{
	Rotate<LEFT>(root, x);
}

void RotateRight(Node*& root, Node* x) noexcept
{
	Rotate<RIGHT>(root, x);
}

void PostInsertRebalance(Node*& root, Node* node) noexcept
{
	while (IsRed(node->parent))
	{
		auto* grand_parent = node->parent->parent;
		if (node->parent == grand_parent->children[LEFT])
		{
			Node* uncle = grand_parent->children[RIGHT];
			if (IsRed(uncle))
			{
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grand_parent->color = RED;
				node = grand_parent;
			}
			else
			{
				if (node == node->parent->children[RIGHT])
				{
					node = node->parent;
					RotateLeft(root, node);
				}

				node->parent->color = BLACK;
				grand_parent->color = RED;
				RotateRight(root, grand_parent);
			}
		}
		else
		{
			Node* uncle = grand_parent->children[LEFT];
			if (IsRed(uncle))
			{
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grand_parent->color = RED;
				node = grand_parent;
			}
			else
			{
				if (node == node->parent->children[LEFT])
				{
					node = node->parent;
					RotateRight(root, node);
				}

				node->parent->color = BLACK;
				grand_parent->color = RED;
				RotateLeft(root, grand_parent);
			}
		}
	}

	root->color = BLACK;
}

void PostRemoveRebalance(Node*& root, Node* x) noexcept
{
	while (x != root && IsBlack(x))
	{
		if (x == x->parent->children[LEFT])
		{
			Node* w = x->parent->children[RIGHT];
			if (IsRed(w))
			{
				w->color = BLACK;
				x->parent->color = RED;
				RotateLeft(root, x->parent);
				w = x->parent->children[RIGHT];
			}

			if (IsBlack(w->children[LEFT]) && IsBlack(w->children[RIGHT]))
			{
				w->color = RED;
				x = x->parent;
			}
			else
			{
				if (IsBlack(w->children[RIGHT]))
				{
					w->children[LEFT]->color = BLACK;
					w->color = RED;
					RotateRight(root, w);
					w = x->parent->children[RIGHT];
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->children[RIGHT]->color = BLACK;
				RotateLeft(root, x->parent);
				x = root;
			}
		}
		else
		{
			Node* w = x->parent->children[LEFT];
			if (IsRed(w))
			{
				w->color = BLACK;
				x->parent->color = RED;
				RotateRight(root, x->parent);
				w = x->parent->children[LEFT];
			}

			if (IsBlack(w->children[LEFT]) && IsBlack(w->children[RIGHT]))
			{
				w->color = RED;
				x = x->parent;
			}
			else
			{
				if (IsBlack(w->children[LEFT]))
				{
					w->children[RIGHT]->color = BLACK;
					w->color = RED;
					RotateLeft(root, w);
					w = x->parent->children[LEFT];
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->children[LEFT]->color = BLACK;
				RotateRight(root, x->parent);
				x = root;
			}
		}
	}

	x->color = BLACK;
}

void Transplant(Node*& root, Node* u, Node* v) noexcept
{
	if ( u->parent == Node::SENTINEL() )
	{
		root = v;
	}
	else if (u == u->parent->children[LEFT])
	{
		u->parent->children[LEFT] = v;
	}
	else
	{
		u->parent->children[RIGHT] = v;
	}

	v->parent = u->parent;
}

auto Minimum(Node* node) noexcept -> Node*
{
	while (node->children[LEFT] != Node::SENTINEL())
		node = node->children[LEFT];
	return node;
}

auto Maximum(Node* node) noexcept -> Node*
{
	while (node->children[RIGHT] != Node::SENTINEL())
		node = node->children[RIGHT];
	return node;
}

void DebugPrint(Node* node, size_t indent) noexcept
{
	if (!node || node == Node::SENTINEL())
		return;

	DebugPrint(node->right(), indent + 1);

	for (int i = 0; i < indent; i++)
	{
#ifdef _WIN32
		char buf[5];
		sprintf_s(buf, "%*s", 4, "");
		OutputDebugStringA(buf);
#else
		std::cout << "	  ";
#endif
	}

	if (node->color == RED)
	{
#ifdef _WIN32
		char buf[256];
		sprintf_s(buf, "R %llu\n", node->size);
		OutputDebugStringA(buf);
#else
		std::cout << "R " << node->size << "\n";
#endif
	}
	else
	{
#ifdef _WIN32
		char buf[256];
		sprintf_s(buf, "B %llu\n", node->size);
		OutputDebugStringA(buf);
#else
		std::cout << "B " << node->size << "\n";
#endif
	}

	DebugPrint(node->left(), indent + 1);
}

auto ValidateRBTBlackHeights(Node* root) noexcept -> bool
{
	std::vector<size_t> blackHeights;
	struct visitor
	{
		static void visit(Node* node, size_t blackHeight, std::vector<size_t>& heights) noexcept
		{
			if (!node)
				return;

			if (node->left())
				visit(node->left(), blackHeight + ( 1 - node->left()->color ), heights);

			if (node->right())
				visit(node->right(), blackHeight + ( 1 - node->right()->color ), heights);

			if (!node->left() && !node->right())
			{
				heights.push_back(blackHeight);
			}
		}
	};

	visitor::visit(root, 1, blackHeights);

	const auto result = std::ranges::adjacent_find(blackHeights, std::not_equal_to<>());
	return result == blackHeights.end();
}

auto ValidateRBTRoot(const Node* root) noexcept -> bool
{
	return IsBlack(root);
}

auto ValidateRBTOrdering(Node* root) noexcept -> bool
{
	std::vector<size_t> values;
	struct visitor
	{
		static void visit(Node* node, std::vector<size_t>& values) noexcept
		{
			if (!node || node == Node::SENTINEL())
				return;

			if (node->left())
				visit(node->left(), values);

			values.push_back(node->size);

			if (node->right())
				visit(node->right(), values);
		}
	};

	visitor::visit(root, values);

	const auto result = std::ranges::adjacent_find(values, std::greater<>());
	return result == values.end();
}

auto ValidateRBTSentinels() noexcept -> bool
{
	return IsBlack(Node::SENTINEL());
}

auto ValidateRBTRedChildren(Node* root) noexcept -> bool
{
	std::vector<bool> values;
	struct visitor
	{
		static void visit(Node* node, std::vector<bool>& redsHaveBlackChildren) noexcept
		{
			if (!node || node == Node::SENTINEL())
				return;

			if (node->left())
				visit(node->left(), redsHaveBlackChildren);

			if (IsRed(node))
			{
				const bool hasTwoBlackChildren = IsBlack(node->left()) && IsBlack(node->right());
				redsHaveBlackChildren.push_back(hasTwoBlackChildren);
			}

			if (node->right())
				visit(node->right(), redsHaveBlackChildren);
		}
	};

	visitor::visit(root, values);

	constexpr auto is_true = [](bool b){ return b; };
	return std::ranges::all_of(values, is_true);
}

auto ValidateRBProperties(Node* root) noexcept -> bool
{
	return ValidateRBTBlackHeights(root) 
		&& ValidateRBTRoot(root) 
		&& ValidateRBTOrdering(root) 
		&& ValidateRBTSentinels() 
		&& ValidateRBTRedChildren(root);
}

void ListInsert(Node*& head, Node* node) noexcept
{
	assert(node && "Node to insert is a nullptr");
	assert(node->prev == nullptr && "Node should be blank and have no prev or next");
	assert(node->next == nullptr && "Node should be blank and have no prev or next");

	if (!head)
	{
		head = node;
	}
	else if (node < head)
	{
		assert(head->prev == nullptr && "Head has a valid prev pointer");
		node->next = head;
		head->prev = node;

		head = node;
	}
	else
	{
		Node* curr = head;
		while (true)
		{
			const uintptr_t node_address = ptr_to_address(node);
			const uintptr_t curr_address = ptr_to_address(curr);

			if (node_address < curr_address)
				break;

			if (curr->next == nullptr)
				break;

			curr = curr->next;
		}

		if (node < curr)
		{
			node->prev = curr->prev;
			node->next = curr;
			
			if (curr->prev)
				curr->prev->next = node;

			curr->prev = node;
		}
		else
		{
			assert(curr < node && "Appending to the tail but the the node is less than curr");
			node->prev = curr;
			curr->next = node;
		}
	}
}

void ListRemove(Node*& head, Node* node) noexcept
{
	if (node == head)
	{
		assert(node->prev == nullptr);
		head = std::exchange(head->next, node->next);
	}
	else
	{
		node->prev->next = node->next;
	}

	if (node->next)
	{
		node->next->prev = node->prev;
	}
}

auto ValidateListProperties(Node* head) noexcept -> bool
{
	return ValidateListOrdering(head) && ValidateListLinks(head);
}

auto ValidateListOrdering(Node* head) noexcept -> bool
{
	if (!head)
		return true;

	std::vector<size_t> values;
	Node* curr = head;
	while (curr)
	{
		values.push_back(wmcv::ptr_to_address(curr));
		curr = curr->next;
	}

	const auto result = std::ranges::adjacent_find(values, std::greater<>());
	return result == values.end();
}

auto ValidateListLinks(Node* head) noexcept -> bool
{
	Node* curr = head;
	while (curr)
	{
		if (curr->prev && curr->prev->next != curr)
			return false;

		if (curr->next && curr->next->prev != curr)
			return false;

		curr = curr->next;
	}

	return true;
}
} // namespace wmcv