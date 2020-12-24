#ifndef APE_THREAD_QUEUE_HPP
#define APE_THREAD_QUEUE_HPP
#include <ape/config.hpp>

#include <type_traits>
#include <mutex>
#include <deque>
#include <condition_variable>


BEGIN_APE_NAMESPACE
namespace thread{

    template<typename T>
    class queue {
        
    public:
        typedef std::chrono::steady_clock::duration timeout_type;
        typedef T value_type;
        typedef std::deque<value_type> queue_type;
        
        /// default ctor
        /// \post isEmpty()
        queue() = default;
        
        /// dtor
        ~queue() = default;
        
        queue(const queue&) = delete;
        queue& operator=(const queue&) = delete;
        
        void stop() {
            auto lock = lock_self_();
            m_stopped = true;
            m_condition_variable.notify_all();
        }

		bool is_stopped() const {
            auto lock = lock_self_();
            return m_stopped;
		}
        
        /// \return the size of the queue
		/// \note sould not depends on this function, debug purpose
        size_t size() const {
            auto lock = lock_self_();
            return m_queue.size();
        }
        
        /// append en element at the end of the queue
        /// \param msg enqueue element
        /// \note if some threads are waiting on the queue, at least one would be woked up. Be care of spurious wakeup issuse.
        /// \note it'll call notify after pushed the given message
        /// \return bool indicating appending success or failure, with failure usually indicating a m_stopped queue
        bool push_back(const value_type& msg) {
            auto lock = lock_self_();
            if (m_stopped) return false;
            m_queue.push_back(msg);
            notify_();
            return true;
        }
        
        /// append the given element at the end of the queue.
        /// \param args arguments to forward to the constructor of the element
        /// \note it'll call notify after pushed the given message
        /// \return bool indicating appending success or failure, with failure usually indicating a m_stopped queue
        template<typename ... Args>
        bool emplace_back(Args&& ... args) {
            auto lock = lock_self_();
            if (m_stopped) return false;
            m_queue.emplace_back(std::forward<Args>(args)...);
            notify_();
            return true;
        }
        
        /// insert a priority element, the given message will be dispatched before any message existed in queue
        /// \param msg given element
        /// \note it'll call notify after pushed the given message
        /// \return bool indicating insertion success or failure, with failure usually indicating a m_stopped queue
        bool push_front(const value_type& msg) {
            auto lock = lock_self_();
            if (m_stopped) return false;
            m_queue.push_front(msg);
            notify_();
            return true;
        }
        
        /// move and insert a priority element, the given message will be dispatched before any message existed in queue
        /// \param msg given element
        /// \note it'll call notify after pushed the given message
        /// \return bool indicating insertion success or failure, with failure usually indicating a m_stopped queue
        template<typename ... Args>
        bool emplace_front(Args&& ... args) {
            auto lock = lock_self_();
            if (m_stopped) return false;
            m_queue.emplace_front(std::forward<Args>(args)...);
            notify_();
            return true;
        }
        
        /// return true if the queue is empty
        /// \note the result is volatile, debug purpose
        bool is_empty() const {
            auto lock = lock_self_();
            return m_queue.empty();
        }
        
        /// wait until queue is not empty for an indefinite long duration or until queue is m_stopped
        /// \return true if queue is both non-empty and non-m_stopped
#if CPP_STANDARD >= CPP_STD_20
		[[nodiscard]]
#endif
        bool wait() {
            auto lock = lock_self_();

            m_condition_variable.wait(lock,
                                    [&]{
                                        return m_stopped || !m_queue.empty();
                                    });
            return !m_queue.empty() && !m_stopped;
        }
        
        /// wait utill \ref awake() is called by other thread, or expires
        /// \param timeout time of expiration
        /// \return true if queue is both non-empty and non-m_stopped
        /// \note ideally, an \ref awake() calling should only wake up one waiter, but it's not guaranteed due to spurious wakeup issuse.
#if CPP_STANDARD >= CPP_STD_20
		[[nodiscard]]
#endif
        bool wait(timeout_type timeout) {
            auto lock = lock_self_();
            m_condition_variable.wait_for(lock,
                                        timeout,
                                        [&]{
                                            return m_stopped || !m_queue.empty();
                                        });
            return !m_queue.empty() && !m_stopped;
        }
        

        
        /// wait till queue is not empty for an indefinite long duration and pop the front message from the queue and returns true. In case where the queue is m_stopped, the function returns false.
        /// \param output the popped message is stored in output
        /// \return bool indicating the pop is success or fail
        /// \note invalid message doesn't means the queue is empty.
        bool wait_and_pop(value_type& output) {
            auto lock = lock_self_();
            m_condition_variable.wait(lock,
                                    [&]{
                                        return m_stopped || !m_queue.empty();
                                    });

            return pop_front_if_not_empty_(output);
        }
        
        /// wait till queue is not empty for a specified long duration and pop the front message from the queue and returns true. In case where the queue is m_stopped, the function returns false.
        /// \param output the popped message is stored in output
        /// \param timeout the longest duration of time to wait
        /// \return bool indicating the pop is success or fail
        /// \note invalid message doesn't means the queue is empty.
        bool wait_and_pop(value_type& output, timeout_type timeout) {
            auto lock = lock_self_();
            m_condition_variable.wait_for(lock,
                                        timeout,
                                        [&]{
                                            return m_stopped || !m_queue.empty();
                                        });
            
            return pop_front_if_not_empty_(output);
        }
        
        /// pop the front message from the queue and returns true. In case where the queue is empty or in m_stopped status, the function returns false.
        /// \param output the popped message is stored in output
        /// \return bool indicating the pop is success or fail
        /// \note invalid message doesn't means the queue is empty.
        bool try_pop(value_type&  output) {
            auto lock = lock_self_();
            return pop_front_if_not_empty_(output);
        }
        
        
        /// \return get message in the front of queue, but won't remove it from the queue. if the queue is empty, return invalid message.
        /// \note invalid message means the queue is empty, but the result is volatile. debug purpose
        value_type peek() const {
            auto lock = lock_self_();
            return m_queue.empty() ? value_type() : m_queue.front();
        }
        
        /// transfer all the elements in queue to the specified output queue
        /// \param output the output queue to transfer all elements into
        /// \return false if queue is m_stopped, true otherwise
        bool take_all(queue_type& output){
            output.clear();
            auto lock = lock_self_();
            if (m_stopped) return false;
            output.swap(m_queue);
            
            return true;
        }
        
        /// clear all the content in the queue
        void clear(){
            auto lock = lock_self_();
            m_queue.clear();
        }
        
        /// clear all the content in the queue and set the queue to be in non-m_stopped status
        void reset(){
            auto lock = lock_self_();
            m_queue.clear();
            m_stopped = false;
        }
    private:
        std::deque<value_type> m_queue;
        mutable std::mutex m_mutex;
        std::condition_variable m_condition_variable;
        bool m_stopped{ false };

        std::unique_lock<std::mutex> lock_self_() const {
            return std::unique_lock<std::mutex>(m_mutex);
        }

        void notify_() { m_condition_variable.notify_one(); }

        bool pop_front_if_not_empty_(value_type& output) {
            if (m_stopped || m_queue.empty())
                return false;

            output = std::move(m_queue.front());
            m_queue.pop_front();
            return true;
        }
    };

}

END_APE_NAMESPACE

#endif //end APE_THREAD_QUEUE_HPP

