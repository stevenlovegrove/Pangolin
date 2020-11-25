#pragma once
#include <cstdlib>
#include <vector>

namespace fteng
{
	namespace details
	{
		struct conn_base;
		struct sig_base
		{
			struct call
			{
				void* object;
				void* func;
			};

			// space can be optimized by using "struct of array" containers since both always have the same size
			mutable std::vector<call>       calls;
			mutable std::vector<conn_base*> conns;

			// space can be optimized by stealing 2 unused bits from the vector size
			mutable bool calling = false;
			mutable bool dirty = false;

			sig_base() = default;
			~sig_base();
			sig_base(const sig_base&) = delete;
			sig_base& operator= (const sig_base&) = delete;
			sig_base(sig_base&& other) noexcept;
			sig_base& operator= (sig_base&& other) noexcept;
		};

		struct blocked_connection
		{
			const sig_base* sig = nullptr;
			sig_base::call call = {nullptr, nullptr};
		};

		struct conn_base
		{
			union
			{
				const sig_base* sig;
				blocked_connection* blocked_conn;
			};
			
			size_t          idx;

			// space can be optimized by stealing bits from index as it's impossible to support max uint64 number of slots
			bool blocked = false;
			bool owned = false;

			conn_base(const sig_base* sig, size_t idx) : sig(sig), idx(idx) {}

			virtual ~conn_base()
			{
				if (!blocked)
				{
					if (sig)
					{
						sig->calls[idx].object = nullptr;
						sig->calls[idx].func = nullptr;
						sig->conns[idx] = nullptr;
						sig->dirty = 1;
					}
				}
				else
				{
					delete blocked_conn;
				}
			}

			void set_sig(const sig_base* sig) 
			{
				if (blocked) this->blocked_conn->sig = sig;
				else this->sig = sig;
			}

			void block()
			{
				if (!blocked)
				{
					blocked = 1;
					const sig_base* orig_sig = sig;
					sig = nullptr;
					blocked_conn = new blocked_connection;
					blocked_conn->sig = orig_sig;
					std::swap(blocked_conn->call, orig_sig->calls[idx]);
				}
			}

			void unblock()
			{
				if (blocked)
				{
					const sig_base* orig_sig = blocked_conn->sig;
					std::swap(blocked_conn->call, orig_sig->calls[idx]);
					delete blocked_conn;
					blocked_conn = nullptr;
					sig = orig_sig;
					blocked = 0;
				}
			}
		};

		template<typename T>
		struct conn_nontrivial : conn_base
		{
			using conn_base::conn_base;

			virtual ~conn_nontrivial()
			{
				if (sig)
					reinterpret_cast<T*>(&sig->calls[idx].object)->~T();
			}
		};

		inline sig_base::~sig_base()
		{
			for (conn_base* c : conns)
			{
				if (c) 
				{
					if (c->owned)
						c->set_sig(nullptr);
					else
						delete c;
				}
			}
		}

		inline sig_base::sig_base(sig_base&& other) noexcept
			: calls(std::move(other.calls))
			, conns(std::move(other.conns))
			, calling(other.calling)
			, dirty(other.dirty)
		{
			for (conn_base* c : conns)
				if (c) c->set_sig(this);
		}

		inline sig_base& sig_base::operator= (sig_base&& other) noexcept
		{
			calls = std::move(other.calls);
			conns = std::move(other.conns);
			calling = other.calling;
			dirty = other.dirty;
			for (conn_base* c : conns)
				if (c) c->set_sig(this);
			return *this;
		}
	}
	
	template<typename F> struct signal;

	// A connection without auto disconnection
	struct connection_raw
	{
		details::conn_base* ptr = nullptr;
	};

	struct connection
	{
		details::conn_base* ptr = nullptr;

		void disconnect()
		{
			delete ptr;
			ptr = nullptr;
		}

		void block()
		{
			ptr->block();
		}

		void unblock()
		{
			ptr->unblock();
		}

		connection() = default;

		~connection()
		{
			disconnect();
		}
		connection(const connection&) = delete;

		connection& operator=(const connection&) = delete;

		connection(connection&& other) noexcept
			: ptr (other.ptr)
		{
			other.ptr = nullptr;
		}

		connection& operator=(connection&& other) noexcept
		{
			disconnect();
			ptr = other.ptr;
			other.ptr = nullptr;
			return *this;
		}

		connection(connection_raw conn) : ptr(conn.ptr)
		{
			ptr->owned = true;
		}
	};

	template<typename ... A>
	struct signal<void(A...)> : details::sig_base
	{
		template<typename ... ActualArgsT>
		void operator()(ActualArgsT&& ... args) const
		{
			bool recursion = calling;
			if (!calling) calling = 1;
			for (size_t i = 0, n = calls.size(); i < n; ++i)
			{
				auto& cb = calls[i];
				if (cb.func)
				{
					if (cb.object == cb.func)
						reinterpret_cast<void(*)(A...)>(cb.func)(std::forward<ActualArgsT>(args)...);
					else
						reinterpret_cast<void(*)(void*, A...)>(cb.func)(&cb.object, std::forward<ActualArgsT>(args)...);
				}
			}

			if (!recursion)
			{
				calling = 0;

				if (dirty)
				{
					dirty = 0;
					//remove all empty slots while patching the stored index in the connection
					size_t sz = 0;
					for (size_t i = 0, n = conns.size(); i < n; ++i)
					{
						if (conns[i]) {
							conns[sz] = conns[i];
							calls[sz] = calls[i];
							conns[sz]->idx = sz;
							++sz;
						}
					}
					conns.resize(sz);
					calls.resize(sz);
				}
			}
		}

		template<auto PMF, class C>
		connection_raw connect(C* object) const
		{
			size_t idx = conns.size();
			auto& call = calls.emplace_back();
			call.object = object;
			call.func = reinterpret_cast<void*>(+[](void* obj, A ... args) {((*reinterpret_cast<C**>(obj))->*PMF)(args...); });
			details::conn_base* conn = new details::conn_base(this, idx);
			conns.emplace_back(conn);
			return { conn };
		}

		template<auto func>
		connection_raw connect() const
		{
			return connect(func);
		}

		connection_raw connect(void(*func)(A...)) const
		{
			size_t idx = conns.size();
			auto& call = calls.emplace_back();
			call.func = call.object = reinterpret_cast<void*>(func);
			details::conn_base* conn = new details::conn_base(this, idx);
			conns.emplace_back(conn);
			return { conn };
		}

		template<typename F>
		connection_raw connect(F&& functor) const
		{
			using f_type = std::remove_pointer_t<std::remove_reference_t<F>>;
			if constexpr (std::is_convertible_v<f_type, void(*)(A...)>)
			{
				return connect(+functor);
			}
			else if constexpr (std::is_lvalue_reference_v<F>)
			{
				size_t idx = conns.size();
				auto& call = calls.emplace_back();
				call.func = reinterpret_cast<void*>(+[](void* obj, A ... args) { (*reinterpret_cast<f_type**>(obj))->operator()(args...); });
				call.object = &functor;
				details::conn_base* conn = new details::conn_base(this, idx);
				conns.emplace_back(conn);
				return { conn };
			}
			else if constexpr (sizeof(std::remove_pointer_t<f_type>) <= sizeof(void*))
			{
				//copy the functor.
				size_t idx = conns.size();
				auto& call = calls.emplace_back();
				call.func = reinterpret_cast<void*>(+[](void* obj, A ... args) { reinterpret_cast<f_type*>(obj)->operator()(args...); });
				new (&call.object) f_type(std::move(functor));
				using conn_t = std::conditional_t<std::is_trivially_destructible_v<F>, details::conn_base, details::conn_nontrivial<F>>;
				details::conn_base* conn = new conn_t(this, idx);
				conns.emplace_back(conn);
				return { conn };
			}
			else
			{
				struct unique
				{
					f_type* ptr;

					unique(f_type* ptr) : ptr(ptr) {}
					unique(const unique&) = delete;
					unique(unique&&) = delete;

					~unique()
					{
						delete ptr;
					}
				};

				size_t idx = conns.size();
				auto& call = calls.emplace_back();
				call.func = reinterpret_cast<void*>(+[](void* obj, A ... args) { reinterpret_cast<unique*>(obj)->ptr->operator()(args...); });
				new (&call.object) unique{ new f_type(std::move(functor)) };
				details::conn_base* conn = new details::conn_nontrivial<unique>(this, idx);
				conns.emplace_back(conn);
				return { conn };
			}
		}
	};
}

