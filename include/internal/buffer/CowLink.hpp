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
template<typename Element>
concept CowNode = std::is_copy_constructible_v<Element> && std::is_default_constructible_v<Element>;

/**
 * @brief The node of buffer
 */
// TODO: support remove unused node
template<typename Element>
requires CowNode<Element>
class CowLink {
public:
	class CowIterator;
	class ConstIterator;

	using NodeContainer = std::vector<std::shared_ptr<Element>>;
	using NodeContainerIndexer = typename NodeContainer::size_type;
	using iterator = CowIterator;
	using const_iterator = ConstIterator;
	constexpr static unsigned defaultNodeSize = 1;

public:
	class ConstIterator {
	public:
		ConstIterator(CowLink *link, NodeContainerIndexer index) : _link{link}, m_current{index} {}
		~ConstIterator() = default;

		using iterator_category = std::random_access_iterator_tag;
		using value_type = const Element;
		using reference_type = const Element &;
		using pointer_type = std::shared_ptr<const Element>;
		using difference_type = NodeContainerIndexer;

		std::weak_ordering operator<=>(const ConstIterator &other) const { return m_current <=> other.m_current; }
		ConstIterator &operator++()
		{
			++m_current;
			return *this;
		}
		reference_type operator*() { return *(_link->m_nodes[m_current].get()); }
		pointer_type operator->() { return std::const_pointer_cast<value_type>(_link->m_nodes[m_current]); }

	private:
		CowLink *_link;
		NodeContainerIndexer m_current;
	};

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

	iterator cowBegin(NodeContainerIndexer index) { return CowLink::CowIterator(this, index, true); }
	iterator cowEnd(NodeContainerIndexer index) { return CowLink::CowIterator(this, index, false); }
	const_iterator constBegin(NodeContainerIndexer index) { return CowLink::ConstIterator(this, index); }
	const_iterator constEnd(NodeContainerIndexer index) { return CowLink::ConstIterator(this, index); }

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
