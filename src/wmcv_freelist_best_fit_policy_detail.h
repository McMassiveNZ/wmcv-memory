#ifndef WMCV_FREELIST_BEST_FIT_POLICY_DETAIL_H_INCLUDED
#define WMCV_FREELIST_BEST_FIT_POLICY_DETAIL_H_INCLUDED

namespace wmcv::detail
{
	inline constexpr size_t LEFT  = 0;
	inline constexpr size_t RIGHT = 1;

	struct Node
	{
		size_t size : 63;
		size_t color : 1;

		Node* children[2];
		Node* parent;

		Node* prev;
		Node* next;

		auto left()  noexcept -> Node* { return children[LEFT]; }
		auto right() noexcept -> Node* { return children[RIGHT]; }

		static Node* SENTINEL() noexcept
		{ 
			static Node sentinel{};
			return &sentinel;
		}
	};

	void Insert(Node*& root, Node* node) noexcept;
	void Remove(Node*& root, Node* node) noexcept;

	auto IsRed(const Node* node) noexcept -> bool;
	auto IsBlack(const Node* node) noexcept -> bool;

	void RotateLeft(Node*& root, Node* node) noexcept;
	void RotateRight(Node*& root, Node* node) noexcept;

	void PostInsertRebalance(Node*& root, Node* node) noexcept;
	void PostRemoveRebalance(Node*& root, Node* node) noexcept;

	void Transplant(Node*& root, Node* u, Node* v) noexcept;
	auto Minimum(Node* node) noexcept -> Node*;
	auto Maximum(Node* node) noexcept -> Node*;

	void DebugPrint(Node* root, size_t indent = 0) noexcept;
	auto ValidateRBTBlackHeights(Node* root) noexcept -> bool;
	auto ValidateRBTRoot(const Node* root) noexcept -> bool;
	auto ValidateRBTOrdering(Node* root) noexcept -> bool;
	auto ValidateRBTSentinels() noexcept -> bool;
	auto ValidateRBTRedChildren(Node* root) noexcept -> bool;
	auto ValidateRBProperties(Node* root) noexcept -> bool;

	void ListInsert(Node*& head, Node* node) noexcept;
	void ListRemove(Node*& head, Node* node) noexcept;

	auto ValidateListProperties(Node* head) noexcept -> bool;
	auto ValidateListOrdering(Node* head) noexcept -> bool;
	auto ValidateListLinks(Node* head) noexcept -> bool;
}

#endif //WMCV_FREELIST_BEST_FIT_POLICY_DETAIL_H_INCLUDED