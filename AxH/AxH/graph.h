#ifndef __H_GRAPH
#define __H_GRAPH

#include <vector>
#include "Data.h"
#include "ErrorHandling.h"
#include "inc_settings.h"

namespace Graph {

	template<typename S, typename D> struct _NOSIGN {
		static S getSignature(const D* const in) {
			return S();
		}
	};

	template<
		typename S,
		typename D,
		class Signer,
		bool forceSigner = false >
		///<summary>
		///Implements an abstract TreeNode/Tree Template.
		///The Nodes are identified by a node_signature, which is obtained by a Signer, which must implement:
		///static S getSignature(const D* const in); and must be able to hndl nullptr values
		///</summary>
		struct NodeTreeBlank {
		protected:
			ptr_vector<NodeTreeBlank> ptrs;
			wrap_ptr<D> ptr = 0;
			S signature;

		public:

			NodeTreeBlank(pass_ptr<D>& in) {
				this->ptr = in;
				updateSignature();
			}
			NodeTreeBlank(pass_ptr<D>&& in) : NodeTreeBlank(in) { }

			NodeTreeBlank(pass_ptr<D>& in, const S& sig) {
				this->ptr = in.get();
				this->signature = (forceSigner ? Signer::getSignature(ptr) : sig);
			}
			NodeTreeBlank(pass_ptr<D>&& in, const S& sig) : NodeTreeBlank(in, sig) { }

			NodeTreeBlank(const NodeTreeBlank& ref) {
				this->operator=(ref);
			}
			NodeTreeBlank() {
				updateSignature();
			}
			~NodeTreeBlank() {
				PRINTP("NodeGraphBlank<" + std::string(typeid(D).name()) + "> Deconstructor", 0, CHANNEL_DECONST_DEBUG);
				ptr.discard();
				ptrs.eraseAll();
			}

			/*
			* Deletes the held data pointer and all children.
			*/
			void discard() {
				ptr.discard();
				ptrs.eraseAll();
			}

			void operator=(const NodeTreeBlank& ref) {
				ptrs.eraseAll();

				for (typename std::vector<NodeTreeBlank*>::const_iterator it = ref.getChildren().begin(); it != ref.getChildren().end(); it++) { // @suppress("Type cannot be resolved")
					this->ptrs.push_back(new NodeTreeBlank(**it));
				}

				this->signature = ref.signature;

				ptr.discard();
				if (ref.ptr.valid()) {
					this->ptr = pass_ptr<D>(new D(ref.ptr.getReference()));
				}
			}

			///<summary>
			///Puts the passed Node at the end of the Child list
			///</summary>
			void push_back_node(pass_ptr<NodeTreeBlank>& in) {
				this->ptrs.push_back(in.get());
			}
			///<summary>
			///Puts the passed Node at the end of the Child list
			///</summary>
			void push_back_node(pass_ptr<NodeTreeBlank>&& in) { this->push_back_node(in); }

			///<summary>
			///Returns a vector containing all nodes in the subtree having this node as its root.
			///The elements are put into the following order: ROOT, then the rest in the follwing order : for each child c {iterate(c)}.
			///</summary>
			std::vector<const NodeTreeBlank<S, D, Signer, forceSigner>*> iterate(bool includeRoot = false) const {
				std::vector<const NodeTreeBlank<S, D, Signer, forceSigner>*> vec;
				this->_iterate(vec, includeRoot);
				return std::move(vec);
			}

			///<summary>
			///Returns a vector containing all nodes in the subtree having this node as its root.
			///The elements are put into the following order: ROOT, then the rest in the follwing order : for each child c {iterate(c)}.
			///</summary>
			std::vector<NodeTreeBlank<S, D, Signer, forceSigner>*> iterate(bool includeRoot = false) {
				std::vector<NodeTreeBlank<S, D, Signer, forceSigner>*> vec;
				this->_iterate(vec, includeRoot);
				return vec;
			}

			///<summary>
			///Puts the passed Node at the front of the Child list
			///</summary>
			void push_front_node(pass_ptr<NodeTreeBlank>& in) {
				this->ptrs.insert(this->ptrs.begin(), in.get());
			}

			///<summary>
			///Puts the passed Node at the front of the Child list
			///</summary>
			void push_front_node(pass_ptr<NodeTreeBlank>&& in) {
				this->ptrs.insert(this->ptrs.begin(), in.get());
			}

			///<summary>
			///Puts the passed Node after the node with the passed signature
			///</summary>
			void push_after_node(pass_ptr<NodeTreeBlank>& in, S& sign) {
				this->ptrs.insert(this->ptrs.begin() + getChildIndex(sign), in.get());
			}

			///<summary>
			///Puts the passed Node after the node with the passed signature
			///</summary>
			void push_after_node(pass_ptr<NodeTreeBlank>&& in, S& sign) {
				this->ptrs.insert(this->ptrs.begin() + getChildIndex(sign), in.get());
			}

			///<summary>
			///Returns a refrence to the Data in this node
			///</summary>
			D& getData() {
				return ptr.getReference();
			}
			///<summary>
			///Returns a refrence to the Data in this node
			///</summary>
			const D& getData() const {
				return ptr.getReference();
			}

			///<summary>
			///Returns a const ptr to the nodes data
			///</summary>
			const D* getDataPointer() const {
				return ptr._cpy();
			}

			///<summary>
			///Returns a vector the child nodes, 
			///ownership to the ChildNode Pointers is NOT transmitted!
			///</summary>
			const ptr_vector<NodeTreeBlank>& getChildren() const {
				return ptrs;
			}

			///<summary>
			///Returns a vector the child nodes, 
			///ownership to the ChildNode Pointers is NOT transmitted!
			///</summary>
			ptr_vector<NodeTreeBlank>& getChildren() {
				return ptrs;
			}

			///<summary>
			///Returns the pointer to the child node with the given signature
			///nullptr if no such child exists
			///</summary>
			pass_null_ptr<NodeTreeBlank> extractFromChildrenBySignature(const S& sig) {
				typename std::vector<owner<NodeTreeBlank*>>::iterator it = ptrs.begin(); // @suppress("Type cannot be resolved")
				NodeTreeBlank* match;
				while (it != ptrs.end()) {
					if ((**it).signature == sig) { // @suppress("Field cannot be resolved")
						match = *it;
						ptrs.erase(it);
						return match;
					}
					it++;
				}
				return nullptr;
			}

			///<summary>
			///Returns the pointer to the child node at the given index
			///nullptr if invalid index
			///</summary>
			pass_null_ptr<NodeTreeBlank> extractFromChildrenByIndex(size_t sig) {
				if (sig < ptrs.size()) {
					NodeTreeBlank* elem = ptrs.at(sig);
					ptrs.erase(ptrs.begin() + sig);
					return elem;
				}
				return nullptr;
			}

			///<summary>
			///returns a pointer to the data, while TRANSMITTING ownership away from this struct
			///</summary>
			pass_ptr<D> extractPointer() {
				pass_ptr<D> tr = this->ptr.extract();
				updateSignature();
				return tr;
			}

			///<summary>
			///Sets the data pointer to the passed data.
			///</summary>
			void setPointer(pass_ptr<D>&& in) {
				setPointer(in);
			}

			///<summary>
			///Sets the data pointer to the passed data.
			///</summary>
			void setPointer(pass_ptr<D>& in) {
				this->ptr = in;
				updateSignature();
			}

			///<summary>
			///Returns the index of the first child which has the passed signature 
			///Returns 0xFFFFFFFFU if no child has the given signature
			///</summary>
			uint32 getChildIndex(const S& in) const {
				int32 ind = 0;
				for (typename std::vector<NodeTreeBlank*>::const_iterator it = this->ptrs.begin(); it != this->ptrs.end(); ind++, it++) {
					if ((**it).signature == in) {
						return ind;
					}
				}
#ifdef __DEBUG
				PRINT_ERR("Dubious Event: No child with such signature, returning 0xFFFFFFFFU", PRIORITY_MESSAGE, CHANNEL_GENERAL_DEBUG);
#endif
				return 0xFFFFFFFFU; //BAD?
			}

			///<summary>
			///Updates the signature of this node, by calling the Signer on the currently held object
			///EVEN if there is a nullptr currently set. 
			///</summary>
			__TBR__ void updateSignature() {
				this->signature = Signer::getSignature(this->ptr._cpy());
			}

			///<summary>
			///Returns the signature of this node
			///</summary>
			S getSignature() const {
#ifdef __DEBUG
				if (!this->ptr.valid()) {
					PRINT_ERR("Dubious activity: getSignature despite invalid data entry... FIX THIS", PRIORITY_MESSAGE, CHANNEL_GENERAL_DEBUG)
				}
#endif

				return this->signature;
			}

			///<summary>
			///returns the first node found matching the passed signature
			///This method operates recursively, that is it searches in the complete SubGraph with this graph as a node
			///</summary>
			NodeTreeBlank* getBySignature(S sign) {
				if (this->signature == sign) {
					return this;
				}
				else {
					for (NodeTreeBlank* bl : ptrs) {
						NodeTreeBlank* rt = bl->getBySignature(sign);
						if (rt) {
							return rt;
						}
					}
				}
				return nullptr;
			}
			///<summary>
			///returns the first node found matching the passed signature
			///This method operates recursively, that is it searches in the complete SubGraph with this graph as a node
			///</summary>
			const NodeTreeBlank* getBySignature(S sign) const {
				if (this->signature == sign) {
					return this;
				}
				else {
					for (const NodeTreeBlank* bl : ptrs) {
						const NodeTreeBlank* rt = bl->getBySignature(sign);
						if (rt) {
							return rt;
						}
					}
				}
				return nullptr;
			}

		private:

			void _iterate(std::vector<const NodeTreeBlank<S, D, Signer, forceSigner>*>& vec, bool incdl) const {
				if (incdl) {
					vec.push_back(this);
				}
				for (const NodeTreeBlank* n : this->ptrs) {
					n->_iterate(vec, true);
				}
			}
			void _iterate(std::vector<NodeTreeBlank<S, D, Signer, forceSigner>*>& vec, bool incdl) {
				if (incdl) {
					vec.push_back(this);
				}
				for (NodeTreeBlank* n : this->ptrs) {
					n->_iterate(vec, true);
				}
			}
	};
}
#endif
