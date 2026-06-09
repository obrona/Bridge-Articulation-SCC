#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = std::less<T>>
struct TreapNode {
    static mt19937 rng;
    
    T val;
    int cnt = 1;
    int prior;
    TreapNode<T, Compare> *l = nullptr, *r = nullptr;

    TreapNode(T val): val(val), prior(rng()) {}
};

template <typename T, typename Compare = std::less<T>>
array<TreapNode<T, Compare>*, 2> split(TreapNode<T> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return {nullptr, nullptr};
    if (cmp(t->val, val)) {
        auto [l, r] = split(t->r, val);
        t->r = l;
        return {t, r};
    } else {
        auto [l, r] = split(t->l, val);
        t->l = r;
        return {l, t};
    }
}

// return less than, equal, more than
template<typename T, typename Compare = std::less<T>>
array<TreapNode<T>*, 3> split_equal(TreapNode<T> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return {nullptr, nullptr, nullptr};

    if (!cmp(t->val, val) && !cmp(val, t->val)) {
        auto l = t->l, r = t->r;
        t->l = t->r = nullptr;
        return {t->l, t, t->r};
    }

    if (cmp(t->val, val)) {
        auto [l, e, r] = split_equal(t->r, val, cmp);
        t->r = l;
        return {t, e, r};
    } else {
        auto [l, e, r] = split_equal(t->l, val, cmp);
        t->l = l;
        return {l, e, t};
    }
}

template <typename T, typename Compare = std::less<T>>
TreapNode<T, Compare>* merge(TreapNode<T, Compare> *l, TreapNode<T, Compare> *r, const Compare& cmp = Compare()) {
    if (l == nullptr) return r;
    if (r == nullptr) return l;

    // if l->val and r->val are the same, then l cannot have any right children and r cannot have any left children.
    if (!cmp(l->val, r->val) && !cmp(r->val, l->val)) {
        l->prior = max(l->prior, r->prior);
        l->cnt += r->cnt;
        l->r = r->r;
        delete r;
        return l;
    }

    if (l->prior >= r->prior) {
        l->r = merge(l->r, r, cmp);
        return l;
    } else {
        r->l = merge(l, r->l, cmp);
        return r;
    }
}


template <typename T, typename Compare = std::less<T>>
void insert(TreapNode<T, Compare>*& root, T val, const Compare& cmp = Compare()) {
    auto [l, r] = split<T, Compare>(root, val);
    t = merge(l, merge(new TreapNode(val), r));
}

template <typename T, typename Compare = std::less<T>>
void erase(TreapNode<T, Compare>*& root, T val, const Compare& cmp = Compare()) {
    auto [l, e, r] = split_equal<T, Compare>(root, val);
    t = merge(l, r);
}

template<typename T, typename Compare = std::less<T>>
TreapNode<T, Compare>* unite(TreapNode<T, Compare>* t1, TreapNode<T, Compare>* t2) {
    if (t1 == nullptr) return t2;
    if (t2 == nullptr) return t1;

    if (t1->prior < t2->prior) swap(t1, t2);
    
    auto [l, e, r] = split(t2, t1->val);
    if (e != nullptr) {
        t1->cnt += e->cnt;
        delete e;
    }
    t1->l = unite(t1->l, l);
    t1->r = unite(t1->r, r);
    return t1;
}

template<typename T, typename Compare = std::less<T>>
void clean(TreapNode<T, Compare>* t) {
    if (t == nullptr) return;
    clean(t->l);
    clean(t->r);
    delete t;
}




/*template <typename T, typename Compare = std::less<T>>
struct Treap {
    mt19937 rng;
    Compare cmp;
    TreapNode<T> *root = nullptr;

    bool equal(T t1, T t2) {
        return !cmp(t1, t2) && !cmp(t2, t1);
    } 

    // here left split is values < val, right split is >= val.
    // to follow cpp convention of less than.
    pair<TreapNode<T>*, TreapNode<T>*> split(TreapNode<T> *t, T val) {
        if (t == nullptr) return {nullptr, nullptr};

        if (cmp(t->val, val)) {
            auto [l, r] = split(t->r, val);
            t->r = l;
            return {t, r};
        } else {
            auto [l, r] = split(t->l, val);
            t->l = r;
            return {l, t};
        }
    }

    // the greatest key in l is < smallest key in r
    TreapNode<T>* merge(TreapNode<T> *l, TreapNode<T> *r) {
        if (l == nullptr) return r;
        if (r == nullptr) return l;

        // if l->val and r->val are the same, then l cannot have any right children 
        // and r cannot have any left children
        if (equal(l->val, r->val)) {
            if (l->prior >= r->prior) {
                l->cnt += r->cnt;
                l->r = r->r;
                delete r;
                return l;
            } else {
                r->cnt += l->cnt;
                r->l = l->l;
                delete l;
                return r;
            }
        }
        
        if (l->prior >= r->prior) {
            l->r = merge(l->r, r);
            return l;
        } else {
            r->l = merge(l, r->l);
            return r;
        }
    }

    void insert(T val) {
        auto [tl, tr] = split(root, val);
        auto tnew = new TreapNode<T>(val, rng());
        root = merge(tl, merge(tnew, tr));
    }

    void erase(T val) {
        function<TreapNode<T>*(T, TreapNode<T>*)> helper =
            [&helper, this] (T val, TreapNode<T>* t) -> TreapNode<T>* {
            if (t == nullptr) return nullptr;

            if (cmp(t->val, val)) {
                t->r = helper(val, t->r);
                return t;
            } 
            
            else if (cmp(val, t->val)) {
                t->l = helper(val, t->l);
                return t;
            } 
            
            else {
                if (--(t->cnt) > 0) {
                    return t;
                } else {
                    auto out = merge(t->l, t->r);
                    delete t;
                    return out;
                }
            }
        };

        root = helper(val, root);
    }

    ~Treap() {
        function<void(TreapNode<T>* t)> helper = [&helper] (TreapNode<T>* t) {
            if (t == nullptr) return;
            helper(t->l);
            helper(t->r);
            delete t;
        };

        helper(root);
    }
};*/



//template<typename T, typename Compare = std::less<T>>
