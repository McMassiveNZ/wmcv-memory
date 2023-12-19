#include "test_pch.h"
#include "wmcv_freelist_best_fit_policy_detail.h"
#include "wmcv_memory/wmcv_allocator_utility.h"

TEST(test_bestfit_policy_detail, test_is_black_on_default_node)
{
	wmcv::detail::Node root{};
	EXPECT_TRUE(wmcv::detail::IsBlack(&root));
}

TEST(test_bestfit_policy_detail, test_is_black_on_nullptr)
{
	EXPECT_TRUE(wmcv::detail::IsBlack(nullptr));
}

TEST(test_bestfit_policy_detail, test_color_node_red)
{
	wmcv::detail::Node node{};
	node.color = 1;

	EXPECT_TRUE(wmcv::detail::IsRed(&node));
}

TEST(test_bestfit_policy_detail, test_rotate_left)
{
	/*
	*		x                         y
	*     /   \                      /  \
	*    A     y       ==>          x    G
	*         / \                  / \
	*        B   G                A   B
	*/


	wmcv::detail::Node* root = nullptr;

	wmcv::detail::Node x{};
	wmcv::detail::Node y{};
	wmcv::detail::Node A{};
	wmcv::detail::Node B{};
	wmcv::detail::Node G{};

	root = &x;
	root->parent = wmcv::detail::Node::SENTINEL();

	root->children[wmcv::detail::LEFT] = &A;
	root->children[wmcv::detail::RIGHT] = &y;
	A.parent = y.parent = root;
	
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::LEFT] = &B;
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT] = &G;

	B.parent = G.parent = root->children[wmcv::detail::RIGHT];
	B.children[wmcv::detail::LEFT] = B.children[wmcv::detail::RIGHT] = G.children[wmcv::detail::LEFT] = G.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	wmcv::detail::RotateLeft(root, &x);

	EXPECT_EQ(root, &y);
	EXPECT_EQ(root->left(), &x);
	EXPECT_EQ(root->right(), &G);
	EXPECT_EQ(root->left()->left(), &A);
	EXPECT_EQ(root->left()->right(), &B);
}

TEST(test_bestfit_policy_detail, test_rotate_right)
{
	/*
	*		y                   x
	*      / \                 / \
	*    x    G      =>       A   Y
	*   /  \                     / \
	*  A    B                   B   G
	*/


	wmcv::detail::Node* root = nullptr;

	wmcv::detail::Node x{};
	wmcv::detail::Node y{};
	wmcv::detail::Node A{};
	wmcv::detail::Node B{};
	wmcv::detail::Node G{};

	root = &y;
	root->parent = wmcv::detail::Node::SENTINEL();

	root->children[wmcv::detail::LEFT] = &x;
	root->children[wmcv::detail::RIGHT] = &G;
	x.parent = G.parent = root;

	root->children[wmcv::detail::LEFT]->children[wmcv::detail::LEFT] = &A;
	root->children[wmcv::detail::LEFT]->children[wmcv::detail::RIGHT] = &B;
	A.parent = B.parent = root->children[wmcv::detail::LEFT];
	A.children[wmcv::detail::LEFT] = B.children[wmcv::detail::LEFT] = A.children[wmcv::detail::RIGHT] = B.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	wmcv::detail::RotateRight(root, &y);

	EXPECT_EQ(root, &x);
	EXPECT_EQ(root->left(), &A);
	EXPECT_EQ(root->right(), &y);
	EXPECT_EQ(root->right()->left(), &B);
	EXPECT_EQ(root->right()->right(), &G);
}

TEST(test_bestfit_policy_detail, test_insert)
{
	auto* root = wmcv::detail::Node::SENTINEL();
	std::array<wmcv::detail::Node, 10> nodes{};
	nodes[0].size = 1;
	nodes[1].size = 6;
	nodes[2].size = 8;
	nodes[3].size = 11;
	nodes[4].size = 13;
	nodes[5].size = 15;
	nodes[6].size = 17;
	nodes[7].size = 22;
	nodes[8].size = 25;
	nodes[9].size = 27;

	for (auto& node : nodes)
	{
		wmcv::detail::Insert(root, &node);
	}

	std::stack<wmcv::detail::Node*> in_order;
	wmcv::detail::Node* curr = root;
	decltype(nodes)::size_type numPops = 0;

	while ( curr != wmcv::detail::Node::SENTINEL() || !in_order.empty())
	{
		while ( curr != wmcv::detail::Node::SENTINEL())
		{
			in_order.push(curr);
			curr = curr->children[wmcv::detail::LEFT];
		}

		curr = in_order.top();
		in_order.pop();

		EXPECT_EQ(curr->size, nodes[numPops++].size);

		curr = curr->children[wmcv::detail::RIGHT];
	}

	EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));
}

TEST(test_bestfit_policy_detail, test_post_insert_rebalance_case_1_node_and_parent_are_red_right)
{
	wmcv::detail::Node A{};
	wmcv::detail::Node B{};
	wmcv::detail::Node C{};
	wmcv::detail::Node D{};
	wmcv::detail::Node* root = &C;

	A.size = 2;
	A.color = 1;
	A.parent = &C;
	A.children[wmcv::detail::LEFT] = wmcv::detail::Node::SENTINEL();
	A.children[wmcv::detail::RIGHT] = &B;

	B.size = 4;
	B.color = 1;
	B.parent = &A;
	B.children[wmcv::detail::LEFT] = wmcv::detail::Node::SENTINEL();
	B.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	C.size = 1;
	C.color = 0;
	C.parent = wmcv::detail::Node::SENTINEL();
	C.children[wmcv::detail::LEFT] = &A;
	C.children[wmcv::detail::RIGHT] = &D;

	D.size = 3;
	D.color = 1;
	D.parent = &C;
	D.children[wmcv::detail::LEFT] = wmcv::detail::Node::SENTINEL();
	D.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	EXPECT_FALSE(wmcv::detail::ValidateRBTRedChildren(root));

	wmcv::detail::PostInsertRebalance(root, &B);

	EXPECT_TRUE(wmcv::detail::ValidateRBTRedChildren(root));

	EXPECT_TRUE(IsBlack(&C));
	EXPECT_TRUE(IsBlack(&A));
	EXPECT_TRUE(IsBlack(&D));
	EXPECT_TRUE(IsRed(&B));

	EXPECT_EQ(root, &C);
	EXPECT_EQ(root->left(), &A);
	EXPECT_EQ(root->right(), &D);
	EXPECT_EQ(root->left()->right(), &B);
}

TEST(test_bestfit_policy_detail, test_post_insert_rebalance_case_1_node_and_parent_are_red_left)
{
	wmcv::detail::Node A{};
	wmcv::detail::Node B{};
	wmcv::detail::Node C{};
	wmcv::detail::Node D{};
	wmcv::detail::Node* root = &C;

	A.color = 1;
	A.parent = &B;
	A.children[wmcv::detail::LEFT] = wmcv::detail::Node::SENTINEL();
	A.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	B.color = 1;
	B.parent = &C;
	B.children[wmcv::detail::LEFT] = &A;
	B.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	C.color = 0;
	C.parent = wmcv::detail::Node::SENTINEL();
	C.children[wmcv::detail::LEFT] = &B;
	C.children[wmcv::detail::RIGHT] = &D;

	D.color = 1;
	D.parent = &C;
	D.children[wmcv::detail::LEFT] = wmcv::detail::Node::SENTINEL();
	D.children[wmcv::detail::RIGHT] = wmcv::detail::Node::SENTINEL();

	EXPECT_FALSE(wmcv::detail::ValidateRBTRedChildren(root));

	wmcv::detail::PostInsertRebalance(root, &A);

	EXPECT_TRUE(wmcv::detail::ValidateRBTRedChildren(root));

	EXPECT_TRUE(IsBlack(&C));
	EXPECT_TRUE(IsBlack(&B));
	EXPECT_TRUE(IsBlack(&D));
	EXPECT_TRUE(IsRed(&A));

	EXPECT_EQ(root, &C);
	EXPECT_EQ(root->left(), &B);
	EXPECT_EQ(root->right(), &D);
	EXPECT_EQ(root->left()->left(), &A);
}

TEST(test_bestfit_policy_detail, test_delete)
{
	auto* root = wmcv::detail::Node::SENTINEL();
	std::array<wmcv::detail::Node, 12> nodes{};
	nodes[0].size = 1;
	nodes[1].size = 2;
	nodes[2].size = 3;
	nodes[3].size = 6;
	nodes[4].size = 8;
	nodes[5].size = 11;
	nodes[6].size = 13;
	nodes[7].size = 15;
	nodes[8].size = 17;
	nodes[9].size = 22;
	nodes[10].size = 25;
	nodes[11].size = 27;

	for (auto& node : nodes )
	{
		wmcv::detail::Insert(root, &node);
	}

	EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));

	for ( auto itr = nodes.rbegin(); itr != nodes.rend(); ++itr)
	{
		auto& node = *itr;
		wmcv::detail::Remove(root, &node);

		if (root)
		{
			EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));
		}
	}
}

TEST(test_bestfit_policy_detail, test_insert_same_node_multiple_times)
{
	auto* root = wmcv::detail::Node::SENTINEL();
	std::array<wmcv::detail::Node, 128> nodes{};

	for (auto& node : nodes )
	{
		node.size = 64;
		wmcv::detail::Insert(root, &node);
	}

	std::vector<size_t> blackDepths;
	
	EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));

	for ( auto itr = nodes.rbegin(); itr != nodes.rend(); ++itr)
	{
		auto& node = *itr;
		wmcv::detail::Remove(root, &node);

		if (root)
		{
			EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));
		}
	}
}

TEST(test_bestfit_policy_detail, test_crash_removing_root_in_specific_cast)
{
	std::array<wmcv::detail::Node, 8> nodes{};
	for (auto& node : nodes)
	{
		node.size = 256;
		node.parent = wmcv::detail::Node::SENTINEL();
		node.children[0] = wmcv::detail::Node::SENTINEL();
		node.children[1] = wmcv::detail::Node::SENTINEL();
		node.color = 0;
	}

	auto* root = &nodes[0];
	root->children[wmcv::detail::LEFT] = &nodes[1];
	root->children[wmcv::detail::LEFT]->color = 1;
	root->children[wmcv::detail::LEFT]->parent = root;
	root->children[wmcv::detail::LEFT]->children[wmcv::detail::LEFT] = &nodes[3];
	root->children[wmcv::detail::LEFT]->children[wmcv::detail::LEFT]->parent = root->children[wmcv::detail::LEFT];
	root->children[wmcv::detail::LEFT]->children[wmcv::detail::RIGHT] = &nodes[4];
	root->children[wmcv::detail::LEFT]->children[wmcv::detail::RIGHT]->parent = root->children[wmcv::detail::LEFT];
	root->children[wmcv::detail::RIGHT] = &nodes[2];
	root->children[wmcv::detail::RIGHT]->color = 1;
	root->children[wmcv::detail::RIGHT]->parent = root;
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::LEFT] = &nodes[5];
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT] = &nodes[6];
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::LEFT]->parent = root->children[wmcv::detail::RIGHT];
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->parent = root->children[wmcv::detail::RIGHT];
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT] = &nodes[7];
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->color = 1;
	root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT]->parent = root->children[wmcv::detail::RIGHT]->children[wmcv::detail::RIGHT];

	EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));

	wmcv::detail::Remove(root, root);

	EXPECT_TRUE(wmcv::detail::ValidateRBProperties(root));
}

TEST(test_bestfit_policy_detail, test_list_node_insert_always_append)
{
	std::array<wmcv::detail::Node, 8> nodes{};
	wmcv::detail::Node* head = nullptr;
	for (auto& node : nodes )
	{
		wmcv::detail::ListInsert(head, &node);
	}

	EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
}

TEST(test_bestfit_policy_detail, test_list_node_insert_always_prepend)
{
	std::array<wmcv::detail::Node, 8> nodes{};
	wmcv::detail::Node* head = nullptr;
	for (auto itr = nodes.rbegin(); itr != nodes.rend(); ++itr)
	{
		auto& node = *itr;
		wmcv::detail::ListInsert(head, &node);
	}

	EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
}

TEST(test_bestfit_policy_detail, test_list_node_insert_arbitrary)
{
	std::array<wmcv::detail::Node, 9> nodes{};
	std::array<size_t, 9> indices{4, 3, 5, 2, 7, 1, 6, 0, 8};

	wmcv::detail::Node* head = nullptr;
	for ( size_t i : indices)
	{
		auto* node = &nodes[i];
		wmcv::detail::ListInsert(head, node);
	}

	EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
}

TEST(test_bestfit_policy_detail, test_list_node_remove_always_pop_head)
{
	std::array<wmcv::detail::Node, 8> nodes{};
	wmcv::detail::Node* head = nullptr;
	for (auto& node : nodes )
	{
		wmcv::detail::ListInsert(head, &node);
	}

	for (auto& node : nodes )
	{
		wmcv::detail::ListRemove(head, &node);
		EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
	}
}

TEST(test_bestfit_policy_detail, test_list_node_remove_always_pop_tail)
{
	std::array<wmcv::detail::Node, 8> nodes{};
	wmcv::detail::Node* head = nullptr;
	for (auto& node : nodes )
	{
		wmcv::detail::ListInsert(head, &node);
	}

	for (auto itr = nodes.rbegin(); itr != nodes.rend(); ++itr)
	{
		auto& node = *itr;
		wmcv::detail::ListRemove(head, &node);
		EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
	}
}

TEST(test_bestfit_policy_detail, test_list_node_remove_arbitrary)
{
	std::array<wmcv::detail::Node, 9> nodes{};
	std::array<size_t, 9> indices{4, 3, 5, 2, 7, 1, 6, 0, 8};

	wmcv::detail::Node* head = nullptr;
	for (size_t i : indices)
	{
		auto* node = &nodes[i];
		wmcv::detail::ListInsert(head, node);
	}

	for (size_t i : indices)
	{
		auto* node = &nodes[i];
		wmcv::detail::ListRemove(head, node);
		EXPECT_TRUE(wmcv::detail::ValidateListProperties(head));
	}
}