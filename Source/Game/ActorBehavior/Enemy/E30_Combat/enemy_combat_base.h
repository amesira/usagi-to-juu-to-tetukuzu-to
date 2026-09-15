// enemy_combat_base.h
// 2026/09/10
// ・プレイヤーのPlayerActionBaseとほぼ同じ役割の、敵の戦闘行動の基底クラス
#pragma once

class EnemyContext;

enum class EnemyCombatStatus {
    Running,    // 戦闘行動が継続中
    Success,    // 戦闘行動が成功して終了
    Failure,    // 戦闘行動が失敗して終了
};

/// @brief EnemyCombatTreeが排他的に実行する戦闘行動の基底クラス。
class EnemyCombatBase {
protected:
    int m_priority = 0;
    bool m_isInterruptible = true;

public:
    explicit EnemyCombatBase(int priority = 0, bool isInterruptible = true)
        : m_priority(priority), m_isInterruptible(isInterruptible) {}
    virtual ~EnemyCombatBase() = default;

    virtual void Initialize(EnemyContext& context) {}
    virtual void Finalize(EnemyContext& context) {}

    /// @brief 非実行中の行動を開始できるか。
    virtual bool CanStart(const EnemyContext& context) const = 0;
    /// @brief 非実行中も呼ぶ。再開始クールダウンなどを更新する。
    virtual void UpdateBackground(EnemyContext&, float) {}
    /// @brief 実行中の行動を継続できるか。falseなら割り込み禁止でもCancelする。
    virtual bool CanContinue(const EnemyContext& context) const = 0;
    /// @brief より高優先度の行動に置き換えられるか。攻撃段階に応じてoverrideできる。
    virtual bool IsInterruptible(const EnemyContext&) const { return m_isInterruptible; }
    int GetPriority() const { return m_priority; }

    virtual void Start(EnemyContext& context) = 0;
    virtual EnemyCombatStatus Update(EnemyContext& context, float deltaTime) = 0;
    /// @brief UpdateがSuccessまたはFailureを返したときの終了処理。
    virtual void Finish(EnemyContext& context) = 0;
    /// @brief 継続不可、割り込み、またはConditionによる強制終了の後始末。
    virtual void Cancel(EnemyContext& context) = 0;
};
