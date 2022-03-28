//
// Created by gaojian on 2022/2/23.
//

#ifndef NETPP_COWLINK_HPP
#define NETPP_COWLINK_HPP

#include <vector>
#include <memory>
#include <type_traits>
#include <functional>

namespace netpp::internal::buffer {
/**
 * @brief The node element must has default constructor and can be copy constructed
 * @tparam Element node element
 */
template<typename Element>
concept CowNodeRequire = std::is_copy_constructible_v<Element> && std::is_default_constructible_v<Element>;

/**
 * @brief The buffer container, support copy on write
 * @todo support remove unused node
 */
template<typename Element>
requires CowNodeRequire<Element>
class CowLink {
public:
	class CowIterator;
	class ConstIterator;

	/**
	 * @brief The node container, should support random access
	 */
	using NodeContainer = std::vector<std::shared_ptr<Element>>;
	/**
	 * @brief The position indicator of container
	 */
	using NodeContainerIndexer = typename NodeContainer::size_type;
	/**
	 * @brief Iterator of CowLink, read/write to iteration item are cow
	 */
	using iterator = CowIterator;
	/**
	 * @brief Const iterator of CowLink, readonly
	 */
	using const_iterator = ConstIterator;
	constexpr static unsigned defaultNodeSize = 1;

public:
	/**
	 * @brief Readonly iterator, not cow
	 */
	class ConstIterator {
	public:
		/**
		 * @brief Create an const iterator for CowLink
		 * @param link	cow link
		 * @param index	the index of nodes
		 */
		ConstIterator(CowLink *link, NodeContainerIndexer index) : _link{link}, m_current{index} {}
		~ConstIterator() = default;

		/** @brief Random access iterator */
		using iterator_category = std::random_access_iterator_tag;
		/** @brief Element type */
		using value_type = const Element;
		/** @brief Reference to element type */
		using reference_type = const Element &;
		/** @brief Pointer to element type */
		using pointer_type = std::shared_ptr<const Element>;
		/** @brief Distance of iterator */
		using difference_type = NodeContainerIndexer;

		/**
		 * @brief Compare iterator
		 * @param other iterator to be compared
		 * @return weak order compare
		 */
		std::weak_ordering operator<=>(const ConstIterator &other) const { return m_current <=> other.m_current; }
		/**
		 * @brief Move to next
		 * @return Next iterator
		 */
		ConstIterator &operator++()
		{
			++m_current;
			return *this;
		}
		/**
		 * @brief Dereference iterator
		 * @return Reference to element
		 */
		reference_type operator*() { return *(_link->m_nodes[m_current].get()); }
		/**
		 * @brief Access element
		 * @return Pointer to element
		 */
		pointer_type operator->() { return std::const_pointer_cast<value_type>(_link->m_nodes[m_current]); }

	private:
		CowLink *_link;
		NodeContainerIndexer m_current;
	};

	/**
	 * @brief Iterator that is copy on write, this is the only way to perform write access
	 */
	class CowIterator {
	public:
		/**
		 * Create an iterator for CowLink
		 * @param link 			the link object
		 * @param index 		the item in link
		 * @param cowOnThis 	cow during construction
		 */
		CowIterator(CowLink *link, NodeContainerIndexer index, bool cowOnThis)
				: _link{link}, m_current{index}
		{
			if (cowOnThis)
				cowThisNode();
		}
		~CowIterator() = default;

		using iterator_category = std::random_access_iterator_tag;
		using value_type = Element;
		using reference_type = Element &;
		using pointer_type = std::shared_ptr<Element>;
		using difference_type = NodeContainerIndexer;

		std::weak_ordering operator<=>(const CowIterator &other) const { return m_current <=> other.m_current; }
		CowIterator &operator++()
		{
			++m_current;
			cowThisNode();
			return *this;
		}
		reference_type operator*() { return *(_link->m_nodes[m_current].get()); }
		pointer_type operator->() { return _link->m_nodes[m_current]; }

	private:
		/**
		 * @brief If the node of this iterator is not unique, copy it
		 */
		void cowThisNode()
		{
			if (m_current < _link->size() && !_link->m_nodes[m_current].unique())
				_link->m_nodes[m_current] = std::make_shared<Element>(*(_link->m_nodes[m_current].get()));
		}
		CowLink *_link;
		NodeContainerIndexer m_current;
	};

	CowLink()
	{
		for (NodeContainerIndexer i = 0; i < defaultNodeSize; ++i)
			m_nodes.emplace_back(std::make_shared<Element>());
	}
	CowLink(const CowLink &other) = default;

	/**
	 * @brief Start of copy on write iteration
	 * @param index start at index
	 * @return cow iterator
	 */
	iterator cowBegin(NodeContainerIndexer index) { return CowLink::CowIterator(this, index, true); }
	/**
	 * @brief End of copy on write iteration
	 * @param index end at index
	 * @return cow iterator
	 */
	iterator cowEnd(NodeContainerIndexer index) { return CowLink::CowIterator(this, index, false); }
	/**
	 * @brief Start of readonly iteration
	 * @param index start at index
	 * @return readonly iterator
	 */
	const_iterator constBegin(NodeContainerIndexer index) { return CowLink::ConstIterator(this, index); }
	/**
	 * @brief End of readonly iteration
	 * @param index end at index
	 * @return readonly iterator
	 */
	const_iterator constEnd(NodeContainerIndexer index) { return CowLink::ConstIterator(this, index); }

	/**
	 *
	 */
	[[nodiscard]] NodeContainerIndexer size() const { return m_nodes.size(); }

	/**
	 * move [0, end) to end of the link
	 * @param end the last node to move(not contains this node)
	 */
	void moveToTail(NodeContainerIndexer end, std::function<void(Element &)> moveCallback)
	{
		NodeContainer container;
		for (NodeContainerIndexer i = 0; i < end; ++i)
			container.emplace_back(m_nodes[i]);
		for (NodeContainerIndexer i = end; i < m_nodes.size(); ++i)
			m_nodes[i - end] = std::move(m_nodes[i]);
		for (NodeContainerIndexer i = 0; i < container.size(); ++i)
		{
			moveCallback(*container[i]);
			m_nodes[m_nodes.size() - end + i] = std::move(container[i]);
		}
	}

	/**
	 * @brief Alloc more node in list, after allocation, at least @em length size is available
	 * @param length The minimum size
	 */
	void allocMore(NodeContainerIndexer length)
	{
		if (length < m_nodes.size())
			return;
		length -= m_nodes.size();
//		m_nodes.resize(length);
//		NodeContainerIndexer capacity = m_nodes.capacity();
		for (NodeContainerIndexer i = 0; i < length; ++i)
			m_nodes.emplace_back(std::make_shared<Element>());
	}

private:
	NodeContainer m_nodes;
};
}

#endif //NETPP_COWLINK_HPP
