#include <bits/stdc++.h>
using namespace std;

template <typename T, typename Compare = std::less<T>>
struct TreapNode {
    inline static mt19937 rng{};
    
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

// return less than, equal, more than.
template<typename T, typename Compare = std::less<T>>
array<TreapNode<T>*, 3> split_equal(TreapNode<T> *t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return {nullptr, nullptr, nullptr};

    if (!cmp(t->val, val) && !cmp(val, t->val)) {
        auto l = t->l, r = t->r;
        t->l = t->r = nullptr;
        return {l, t, r};
    }

    if (cmp(t->val, val)) {
        auto [l, e, r] = split_equal(t->r, val, cmp);
        t->r = l;
        return {t, e, r};
    } else {
        auto [l, e, r] = split_equal(t->l, val, cmp);
        t->l = r;
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
void insert(TreapNode<T, Compare>*& t, T val, const Compare& cmp = Compare()) {
    auto [l, r] = split<T, Compare>(t, val);
    t = merge(l, merge(new TreapNode(val), r));
}

template <typename T, typename Compare = std::less<T>>
void erase(TreapNode<T, Compare>*& t, T val, const Compare& cmp = Compare()) {
    auto [l, e, r] = split_equal<T, Compare>(t, val);
    delete e;
    t = merge(l, r);
}

template<typename T, typename Compare = std::less<T>>
TreapNode<T, Compare>* unite(TreapNode<T, Compare>* t1, TreapNode<T, Compare>* t2) {
    if (t1 == nullptr) return t2;
    if (t2 == nullptr) return t1;

    if (t1->prior < t2->prior) swap(t1, t2);
    
    auto [l, e, r] = split_equal(t2, t1->val);
    if (e != nullptr) {
        t1->cnt += e->cnt;
        delete e;
    }
    t1->l = unite(t1->l, l);
    t1->r = unite(t1->r, r);
    return t1;
}

template<typename T, typename Compare = std::less<T>>
int get_cnt(TreapNode<T, Compare>* t, T val, const Compare& cmp = Compare()) {
    if (t == nullptr) return 0;
    if (!cmp(t->val, val) && !cmp(val, t->val)) return t->cnt;
    return cmp(t->val, val) ? get_cnt(t->r, val) : get_cnt(t->l, val);
}

template<typename T, typename Compare = std::less<T>>
void clean(TreapNode<T, Compare>* t) {
    if (t == nullptr) return;
    clean(t->l);
    clean(t->r);
    delete t;
}