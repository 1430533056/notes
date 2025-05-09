#include <ostream>
#include <boost/checked_delete.hpp>
#include <boost/detail/atomic_count.hpp>
#include <iostream>
#include <string>
#include <boost/intrusive_ptr.hpp>

template<class T>
class intrusive_ptr_base {
public:
    intrusive_ptr_base(): ref_count(0) {
        std::cout << "  Default constructor " << std::endl;
    }

    intrusive_ptr_base(intrusive_ptr_base<T> const&): ref_count(0) {
        std::cout << "  Copy constructor..." << std::endl;
    }

    intrusive_ptr_base& operator=(intrusive_ptr_base const& rhs) {
        std::cout << "  Assignment operator..." << std::endl;
        return *this;
    }
     
    friend void intrusive_ptr_add_ref(intrusive_ptr_base<T> const* s) {
        std::cout << "  intrusive_ptr_add_ref..." << std::endl;
        assert(s != 0);
        assert(s->ref_count >= 0);
        ++s->ref_count;
    }
 
    friend void intrusive_ptr_release(intrusive_ptr_base<T> const* s) {
        std::cout << "  intrusive_ptr_release..." << std::endl;
        assert(s != 0);
        assert(s->ref_count > 0);
        if (--s->ref_count == 0)
            boost::checked_delete(static_cast<T const*>(s));
    }

    boost::intrusive_ptr<T> self() {
        return boost::intrusive_ptr<T>((T*)this);
    }

    boost::intrusive_ptr<const T> self() const {
        return boost::intrusive_ptr<const T>((T const*)this);
    }

    int refcount() const {
        return ref_count;
    }

private:
    ///should be modifiable even from const intrusive_ptr objects
    mutable boost::detail::atomic_count ref_count;
 
};

class Connection : public intrusive_ptr_base<Connection> {
public:
    Connection(int id, std::string tag):
        connection_id( id ), connection_tag( tag ) {}

    Connection(const Connection& rhs):
        connection_id( rhs.connection_id ), connection_tag( rhs.connection_tag) {}

    const Connection operator=(const Connection& rhs) {
        if (this != &rhs) {
            connection_id = rhs.connection_id;
            connection_tag = rhs.connection_tag;
        }
        return *this;
    }

private:
    int connection_id;
    std::string connection_tag;
};

int main()
{
    std::cout << "Create an intrusive ptr" << std::endl;
    boost::intrusive_ptr<Connection> con0 (new Connection(4, "sss") );
    std::cout << "Create an intrusive ptr. Refcount = " << con0->refcount() << std::endl;
 
    boost::intrusive_ptr<Connection> con1(con0);
    std::cout << "Create an intrusive ptr. Refcount = " << con1->refcount() << std::endl;
    boost::intrusive_ptr<Connection> con2 = con0;
    std::cout << "Create an intrusive ptr. Refcount = " << con2->refcount() << std::endl;
     
    std::cout << "Destroy an intrusive ptr" << std::endl;
    return 0;
}