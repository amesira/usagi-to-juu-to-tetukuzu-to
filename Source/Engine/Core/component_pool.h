//---------------------------------------------------
// component_pool.h
// 
// ・型ごとにComponentを管理するプール。
// 
// Author：Miu Kitamura
// Date  ：2025/12/07
//---------------------------------------------------
#ifndef COMPONENT_POOL_H
#define COMPONENT_POOL_H

#include <vector>
#include <assert.h>

#include "component_type_id.h"
#include "component_pool_interface.h"

class GameObject;

template <class T>
class ComponentPool : public IComponentPool {
private:
    static constexpr size_t COMPONENTS_MAX = 1024;

    // Componentリスト
    std::vector<T>              m_components = {};

    // GameObjectのIDリスト
    // ・m_componentsとインデックスを対応させる
    std::vector<unsigned int>   m_gameObjectIDs = {};
    // GameObjectIDからComponentへのマッピング
    std::vector<unsigned int>   m_gameObjectIDsToComponents = {};

    // 空きスロット管理用リスト
    std::vector<size_t>         m_freeIndices = {};


public:
    ComponentPool() : IComponentPool(ComponentTypeID::getTypeID<T>(), ComponentTypeID::getBaseTypeID<T>()) {
        m_components.reserve(COMPONENTS_MAX);
        m_gameObjectIDs.reserve(COMPONENTS_MAX);
        m_gameObjectIDsToComponents.resize(COMPONENTS_MAX, UINT_FAST16_MAX);
        m_freeIndices.clear();
    }

    // Componentを生成してComponentPoolに追加
    // ・pGameObject: Componentを所有するGameObjectへのポインタ
    T*      Create(unsigned int gameObjectID) {
        if (gameObjectID >= m_gameObjectIDsToComponents.size()) {
            return nullptr;
        }

        // 1つのGameObjectに同じ型のComponentを複数追加することは禁止する。
        // 重複を許すとIDからComponentへのマッピングが後から追加した要素で
        // 上書きされ、削除時に片方だけがPoolへ残る原因になる。
        const unsigned int existingIndex = m_gameObjectIDsToComponents[gameObjectID];
        if (existingIndex != UINT_FAST16_MAX) {
            assert(false && "A component of the same type already exists on this GameObject.");
            return nullptr;
        }

        assert(m_components.size() < COMPONENTS_MAX && "ComponentPool has reached its maximum capacity.");
        
        // 空きスロットがあればそこを利用
        // ・Componentのメンバ変数に参照型やポインタがある場合、T()でエラーが出る可能性があるため注意
        if (!m_freeIndices.empty()) {
            size_t index = m_freeIndices.back();
            m_freeIndices.pop_back();
            m_components[index] = T();
            m_gameObjectIDs[index] = gameObjectID;
            m_gameObjectIDsToComponents[gameObjectID] = index;
            return &m_components[index];
        }

        m_components.emplace_back();
        m_gameObjectIDs.push_back(gameObjectID);
        m_gameObjectIDsToComponents[gameObjectID] = m_components.size() - 1;

        return &m_components.back();
    }

    // ComponentPoolからComponentを削除
    // ・gameObjectID: 削除するComponentを所有するGameObjectのID
    void    Remove(unsigned int gameObjectID) override {
        if (gameObjectID >= m_gameObjectIDsToComponents.size()) {
            return;
        }

        for (int i = 0; i < m_components.size(); i++) {
            unsigned int id = m_gameObjectIDs[i];
            // 指定されたGameObjectIDと一致したら削除
            if (id == gameObjectID) {
                // 空きスロットとして管理リストに追加
                m_freeIndices.push_back(i);

                m_components[i].SetEnable(false); // 無効化しておく
                m_gameObjectIDs[i] = UINT_FAST16_MAX; // 無効なIDにしておく
                m_gameObjectIDsToComponents[gameObjectID] = UINT_FAST16_MAX; // 無効なインデックスにしておく
                return;
            }
        }
    }

    // GameObjectIDからComponentを取得
    // ・gameObjectID: 取得するComponentを所有するGameObjectのID
    T*  GetByGameObjectID(unsigned int gameObjectID) {
        if (gameObjectID < m_gameObjectIDsToComponents.size()) {
            size_t index = m_gameObjectIDsToComponents[gameObjectID];
            if (index != UINT_FAST16_MAX && index < m_components.size()) {
                return &m_components[index];
            }
        }
        return nullptr;
    }

    // ComponentPool内のComponentリストを取得
    std::vector<T>&     GetList() { return m_components; }

    // ComponentPool内のComponentに対してfuncを呼び出す
    void    Foreach(std::function<void(Component* c)> func) override {
        for (int i = 0; i < m_components.size(); i++) {
            func(&m_components[i]);
        }
    }

    // Component基底クラスの取得
    Component* GetComponentInterface(unsigned int gameObjectID) override {
        return GetByGameObjectID(gameObjectID);
    }

};

#endif
