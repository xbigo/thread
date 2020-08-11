#ifndef APE_THREAD_THREAD_POOL_HPP
#error "This file intends to be included by threadpool.hpp only" 
#endif


BEGIN_APE_NAMESPACE
namespace thread{
	namespace detail {
		template<typename R> struct invoker{
			template<typename F, typename ... Args>
				static void invoke(std::promise<R>& p, F&& f_, Args&&... args_){
					p.set_value(f_(std::forward<Args>(args_)...));
				}
		};

		template<> struct invoker<void> {
			template<typename F, typename ... Args>
				static void invoke(std::promise<void>& p, F&& f_, Args&&... args_){
					f_(std::forward<Args>(args_)...);
					p.set_value();
				}
		};

		template <typename Func, typename ... Args>
		void invoke_imp2(Func& f, Args& ... args) {
			f(std::move(args)...);
		}
		template <typename Pack, std::size_t... Indices>
		void invoke_imp(Pack& pack) {
			invoke_imp2(std::get<Indices>(pack)...);
		}

		template <typename Pack, size_t... Indices>
		 constexpr void invoke2(Pack& pack , std::index_sequence<Indices...>) noexcept {
			invoke_imp<Pack, Indices...>(pack);
		}

		 template <typename  T>
		 struct reference_type {
			 using T0 = std::remove_reference_t<T>;
			 using T1 = std::decay_t<T>;
			 using type = std::conditional_t<std::is_function<T0>::value || std::is_array<T0>::value, T1, std::add_rvalue_reference_t<T1>>;
		 };

		 template <class T>
		 using reference_type_t = typename reference_type<T>::type;
	}

	template<typename F, typename ...Args>
		std::future<detail::result_type<F, Args...>> thread_pool::async(F&& f, Args&&... args) {
			using R = detail::result_type<F, Args...>;

			auto call = [](std::promise<R> p, detail::reference_type_t<F> f_, detail::reference_type_t <Args>... args_){
				try{
					detail::invoker<R>::invoke(p, std::move(f_), std::move(args_)...);
				}
				catch(...){
					p.set_exception(std::current_exception());
				}
			};
			
			

			std::promise<R> p;
			auto future = p.get_future();

			using pack_type = std::tuple<decltype(call), std::promise<R>, std::decay_t<F>, std::decay_t<Args>...>;

			std::shared_ptr<pack_type> pack = std::make_shared< pack_type>(call, std::move(p), std::forward<F>(f), std::forward<Args>(args)...);

			enqueue_([pack] {
				auto& args_  = *pack;
				detail::invoke2(*pack, std::make_index_sequence<3 + sizeof...(Args)>{});
				});

			return future;
		}

}
END_APE_NAMESPACE

