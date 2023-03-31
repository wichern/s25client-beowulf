// Copyright (C) 2005 - 2023 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>

template<class KEY, class VALUE>
class PairingHeap
{
public:
    struct Node
    {
        inline Node(KEY key, const VALUE& value) : key_(key), value_(value) {}

        inline const VALUE& value() const { return value_; }
        inline KEY key() const { return key_; }

        KEY key_;
        VALUE value_;

        Node* child_ = nullptr;
        Node* parent_ = nullptr;
        Node* right_ = nullptr;
    };

    PairingHeap() = default;
    inline ~PairingHeap()
    {
        while(!freeList_.empty())
        {
            delete freeList_.back();
            freeList_.pop_back();
        }
    }

    inline Node* insert(KEY key, const VALUE& value)
    {
        Node* node = alloc(key, value);

        if(root_)
            root_ = pair(root_, node);
        else
            root_ = node;

        size_++;
        return node;
    }

    inline VALUE extract_min(KEY* key = nullptr)
    {
        RTTR_Assert(root_);
        RTTR_Assert(size_ > 0);

        VALUE val = root_->value();
        if(key)
            *key = root_->key();

        Node* h = root_->child_;

        if(0 == h)
        {
            free(root_);
            root_ = 0;
            --size_;
            return val;
        }

        h->parent_ = 0;

        root_->child_ = 0;
        free(root_);

        // First: Merge subtrees in pairs form left to right:
        Node* firstStep = 0;
        Node* h1 = h;
        Node* h2 = h1->right_;
        while(h2)
        {
            h = h2->right_;
            if(firstStep)
            {
                Node* l_res = pair(h1, h2);
                l_res->right_ = firstStep;
                firstStep = l_res;
            } else
            {
                firstStep = pair(h1, h2);
                firstStep->right_ = 0;
            }

            if(h)
            {
                if(h->right_)
                {
                    h2 = h->right_;
                    h1 = h;
                } else
                    h2 = 0;
            } else
                h2 = 0;
        }

        // Second: merge resulting heaps from right to left
        if(h)
        {
            root_ = h;
            h2 = firstStep;
            h = firstStep;
        } else
        {
            root_ = firstStep;
            h2 = firstStep->right_;
            h = h2;
        }

        while(h2)
        {
            h = h->right_;
            h2->right_ = 0;
            root_ = pair(root_, h2);
            h2 = h;
        }

        --size_;

        return val;
    }

    /// Change priority to given key.
    inline void decrease_key(Node* node, KEY key)
    {
        RTTR_Assert(root_);
        RTTR_Assert(node);
        RTTR_Assert(key < node->key());

        node->key_ = key;
        if(node != root_)
        {
            if(node->right_)
                node->right_->parent_ = node->parent_;
            if(node->parent_->child_ == node)
                node->parent_->child_ = node->right_;
            else
                node->parent_->right_ = node->right_;

            node->right_ = 0;

            root_ = pair(root_, node);
        }
    }

    inline bool empty() const { return size_ == 0; }

    inline void clear()
    {
        if(root_)
            clear(root_);
        root_ = nullptr;
        size_ = 0;
    }

private:
    inline Node* alloc(KEY key, VALUE value)
    {
        if(freeList_.empty())
        {
            return new Node(key, value);
        } else
        {
            Node* ret = freeList_.back();
            freeList_.pop_back();
            new(ret) Node(key, value);
            return ret;
        }
    }

    inline void free(Node* obj)
    {
        obj->~Node();
        freeList_.push_back(obj);
    }

    inline void clear(Node* obj)
    {
        if(obj->child_)
            clear(obj->child_);
        if(obj->right_)
            clear(obj->right_);
        free(obj);
    }

    inline Node* pair(Node* left, Node* right)
    {
        if(right->key() < left->key())
        {
            right->parent_ = left->parent_;
            left->parent_ = right;
            left->right_ = right->child_;
            if(left->right_)
                left->right_->parent_ = left;
            right->child_ = left;
            return right;
        } else
        {
            right->parent_ = left;
            left->right_ = right->right_;
            if(left->right_)
                left->right_->parent_ = left;
            right->right_ = left->child_;
            if(right->right_)
                right->right_->parent_ = right;
            left->child_ = right;
            return left;
        }
    }

    Node* root_ = nullptr;
    unsigned size_ = 0;
    std::vector<Node*> freeList_;
};
