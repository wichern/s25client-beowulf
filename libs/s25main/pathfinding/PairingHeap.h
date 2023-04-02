// Copyright (C) 2005 - 2023 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>

#include "nodeObjs/noRoadNode.h"
#include "pathfinding/heap.h"

class PairingHeap : public PriorityHeap
{
public:
    PairingHeap() = default;

    inline void insert(const noRoadNode* node) override
    {
        node->child_ = nullptr;
        node->right_ = nullptr;

        if(root_)
            root_ = pair(root_, node);
        else {
            node->parent_ = nullptr;
            root_ = node;
        }

        // size_++;
    }

    inline const noRoadNode* delete_min() override
    {
        RTTR_Assert(root_);
        // RTTR_Assert(size_ > 0);

        const noRoadNode* ret = root_;

        const noRoadNode* h = root_->child_;

        if(!h)
        {
            root_ = nullptr;
            // --size_;
            return ret;
        }

        h->parent_ = nullptr;
        root_->child_ = nullptr;

        const noRoadNode* firstStep = nullptr;
        const noRoadNode* h1 = h;
        const noRoadNode* h2 = h1->right_;
        while(h2)
        {
            h = h2->right_;
            if(firstStep)
            {
                const noRoadNode* res = pair(h1, h2);
                res->right_ = firstStep;
                firstStep = res;
            } else
            {
                firstStep = pair(h1, h2);
                firstStep->right_ = nullptr;
            }

            if(h)
            {
                if(h->right_)
                {
                    h2 = h->right_;
                    h1 = h;
                } else
                    h2 = nullptr;
            } else
                h2 = nullptr;
        }

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
            h2->right_ = nullptr;
            root_ = pair(root_, h2);
            h2 = h;
        }

        // --size_;

        return ret;
    }

    /// Change priority to given key.
    inline void decrease_key(const noRoadNode* node) override
    {
        RTTR_Assert(root_);
        RTTR_Assert(node);

        if(node != root_)
        {
            if(node->right_)
                node->right_->parent_ = node->parent_;
            if(node->parent_->child_ == node)
                node->parent_->child_ = node->right_;
            else
                node->parent_->right_ = node->right_;

            node->right_ = nullptr;

            root_ = pair(root_, node);
        }
    }

    inline bool empty() const override { return /*size_ == 0*/ nullptr == root_; }

    inline void clear() override
    {
        root_ = nullptr;
        // size_ = 0;
    }

private:
    inline const noRoadNode* pair(const noRoadNode* left, const noRoadNode* right)
    {
        if(right->estimate < left->estimate)
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

    const noRoadNode* root_ = nullptr;
    // unsigned size_ = 0;
};
