#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct TreapNode {
    T val;
    int cnt = 1;
    int prior;
    TreapNode *l = nullptr, *r = nullptr;

    TreapNode(T val, int prior): val(val), prior(prior) {}
};

template <typename T, typename Compare = std::less<T>>
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
   
};
