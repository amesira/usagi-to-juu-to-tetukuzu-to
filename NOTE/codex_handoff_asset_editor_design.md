# MiEngine アセット／編集機能 設計方針（Codex引き継ぎ用）

## この文書の目的

この文書は、MiEngineの次期プロジェクトでCodexが設計・実装を継続するための引き継ぎ資料である。
ここに書かれた内容は完成済み仕様ではなく、今後の実装で維持したい設計方針を示す。

Codexは実装前に対象プロジェクトの現状を確認し、この方針を既存コードへ無理なく適合させること。
Unityの機能をそのまま再現するのではなく、小規模な自作アクションゲーム向けランタイムとして必要な範囲に限定する。

## プロジェクトの基本方針

MiEngineでは、Unityのような汎用シーンエディタを作らない。

対象外とするもの:

- エディタ上で任意のGameObjectを生成・配置する汎用ワークフロー
- 任意のComponentを動的に追加・削除する仕組み
- 完全なPrefabシステム
- ノードベースの汎用Material Editor
- Unity互換を目標とした大規模なParticle/VFX Editor
- 高機能な実行時リフレクションや自動Inspector生成

代わりに、次の役割分担を採用する。

```text
C++       オブジェクト構造、型、安全性、ゲーム固有ロジック
JSON      調整値、アセット参照、制作者が変更するデータ
ImGui     JSONアセットを編集・プレビューする専用UI
Factory   C++で決めた構造へ、JSONの調整値を適用して生成
```

中心概念は、汎用Prefabではなく `Factory + Tuning Asset` である。

```cpp
PlayerFactory::Create(scene, "asset/Data/Player/player_default.player.json");
```

## 共通アセット基盤

DataObject、Material、Particle、VisualEffect、AnimationEventを個別方式で実装せず、先に小さな共通基盤を用意する。

最低限の共通メタデータ:

```cpp
struct AssetHeader
{
    std::string type;
    int formatVersion = 1;
    std::string name;
};

template<class T>
struct AssetHandle
{
    std::string path;
    T* resource = nullptr;
};
```

JSONの基本形:

```json
{
  "type": "ParticleSystem",
  "formatVersion": 1,
  "name": "hit_spark",
  "data": {}
}
```

共通基盤が担当するもの:

- プロジェクトルートからの相対パスでロードする
- 正規化したパスをキーにキャッシュする
- Save、Load、Reloadを提供する
- `formatVersion`を必須とし、将来のデータ移行余地を残す
- 不明なフィールドは可能なら無視し、欠落フィールドにはC++のデフォルト値を使う
- 不正JSON、型不一致、参照切れをログとエディタ上で明示する
- リロード後も利用側のポインタを極力無効化しない
- アセット参照は保存時にはパス、実行時には解決済みポインタとして扱う

初期段階ではGUIDデータベースを作らず、正規化した相対パスをAsset IDとしてよい。
ファイル移動による参照切れが無視できない規模になってからGUIDを検討する。

## 保存データと実行時状態の分離

すべてのアセットで、次の3種類を混同しない。

```text
Desc / Data      JSONに保存できる純粋な設定値とパス
Resource / Asset ロード済み・解決済みの共有データ
Instance         再生時間や一時的Overrideなど、利用箇所ごとの可変状態
```

保存対象へGPUリソースの生ポインタ、GameObjectポインタ、実行中タイマーを入れない。
共有Resourceを演出中に直接変更せず、時間変化や個体差はInstanceへ持たせる。

## DataObjects（ScriptableObject相当）

UnityのScriptableObjectを再現せず、「型付きのゲーム調整用JSON」として実装する。

例:

```cpp
struct PlayerTuningData
{
    float walkSpeed = 5.5f;
    float dashSpeed = 9.0f;
    float acceleration = 18.0f;
    int attackPower = 12;

    AssetHandle<ParticleSystemAsset> hitParticle;
    AssetHandle<GameplayEffectAsset> hitEffect;
};
```

用途:

- Player、Enemyの移動・戦闘パラメータ
- Attack定義
- Dodge定義
- Projectile定義
- カメラや演出の調整値
- 他のMaterial、Particle、Effectへの参照

型ごとに次を明示的に実装する。

```cpp
Serialize(json, data);
Deserialize(json, data);
DrawEditor(data);
```

当面は大規模なリフレクションや完全自動Inspectorを作らない。
型ごとの専用Editorの方が実装量、入力制約、保守性のバランスがよい。

難易度は中。JSON変換よりも、デフォルト値、バージョン、参照解決、リロード時の寿命管理が主な課題になる。

## Material Asset

Materialは次の3層に分ける。

```text
MaterialDesc      Shader名、数値、色、Textureパスなどの保存データ
MaterialResource  ShaderやTextureを解決した共有リソース
MaterialInstance  GameObjectや演出ごとの一時Override
```

`MaterialDesc`の例:

```cpp
struct MaterialDesc
{
    std::string shaderName;
    RenderMode renderMode = RenderMode::Opaque;
    XMFLOAT4 baseColor = { 1, 1, 1, 1 };
    float metallic = 0.0f;
    float roughness = 1.0f;
    std::string albedoTexturePath;
    std::string normalTexturePath;
    std::array<XMFLOAT4, 8> customProperties;
    std::array<std::string, 4> customTexturePaths;
};
```

Material Editorの初期機能:

- Preview用Sphere/Cube/任意Mesh
- Shader選択
- RenderMode、BaseColor、Metallic、Roughness、Emissive、UV編集
- Texture選択
- Custom Property編集
- Save、Save As、Reload

Custom Propertyを単なる番号で見せ続けない。
シェーダーごとに表示名、型、スロット、最小値、最大値などの小さなメタデータを用意する。
初期段階でShader Reflectionやノードエディタは作らず、手書きメタデータでよい。

ホットリロード時はキャッシュ内のオブジェクトそのものを差し替えるより、中身を更新して既存ポインタを維持する。

難易度は中。課題はShaderとのプロパティ整合、RenderState、透過描画順、MaterialInstanceとの責務分離。

## ParticleSystem Asset

Particleはモジュール式の設定データを維持する。

```text
ParticleSystemAsset
  ParticleSystemDesc
    Main
    Emission
    Shape
    SizeOverLifetime
    TextureSheetAnimation
    Renderer

ParticleSystemComponent
  Assetへの参照
  実行用Descコピー
  粒子配列、再生時間、Emitter accumulatorなどの実行時状態
```

ComponentへAssetを設定した時点でDescをコピーし、複数Componentが同じAssetから独立して再生できるようにする。

```cpp
void ParticleSystemComponent::SetAsset(ParticleSystemAsset* asset)
{
    m_asset = asset;
    m_desc = asset->CopyDesc();
    Reset();
}
```

Particle Editorは通常Inspectorではなく専用ウィンドウにする。

初期機能:

- Asset一覧
- Preview Camera、背景、グリッド
- Play、Pause、Stop、Restart
- Save、Save As、Reload
- Main、Emission、Shape、Lifetime、Rendererの編集
- Curve編集
- Texture Sheet Preview
- Alive数、Simulation Time表示

優先して追加するモジュール:

1. Burst
2. Color over Lifetime
3. Velocity / Rotation over Lifetime
4. 必要になった場合のみTrail、Collision、Mesh Particle

最初はCPU Particle、個数上限あり、Additive中心でよい。GPU ParticleやUnity相当の全機能は目標にしない。

最低限のAsset化は難易度中。高度なCurve/Gradient UI、透明ソート、Trail、Collision、GPU化は高難易度。

## VisualEffect Asset（Mesh + Shader演出）

既存または将来の `GameEffectController` と名前・責務を混同しない。

- `GameEffectController` / `GameplayEffect`: HitStop、Camera Shake、FOV、PostEffectなどゲーム全体へ作用する演出
- `VisualEffectAsset` / `MeshEffectAsset`: MeshとMaterialを描画し、時間変化させる視覚オブジェクト

Mesh + Shader系には `VisualEffectAsset` を推奨する。

初期データ:

```cpp
struct VisualEffectAsset
{
    std::string modelPath;
    std::string materialPath;
    float duration = 1.0f;
    bool loop = false;
    FloatCurve scale;
    FloatCurve alpha;
    FloatCurve dissolve;
    FloatCurve emissiveIntensity;
    FloatCurve uvOffsetX;
    FloatCurve uvOffsetY;
};
```

実行時は `VisualEffectComponent` が時間と `MaterialInstance` を持ち、ProcessorがCurveを評価してTransformとMaterialInstanceへ反映する。
共有MaterialResourceは変更しない。

初期スコープ:

- 1 Mesh
- 1 MaterialInstance
- Transform Curve
- Material Custom Property Curve
- Duration、Loop、Play、Stop
- 終了時の自動破棄
- 必要ならBoneへのAttach

Particle、Light、Sound、PostEffectを複数トラックで統合するのは後回しにする。
そこまで進めると小型VFXシーケンサーになるため、別フェーズとして設計する。

基本版は難易度中、複数要素タイムラインは中〜高から高。

## Gameplay Effect Asset

HitStop、Camera Shake、FOV、PostEffect、Particle再生など、攻撃や回避に伴う複合演出をまとめる場合は、VisualEffectとは別のデータにする。

例:

```json
{
  "type": "GameplayEffect",
  "formatVersion": 1,
  "name": "hit_impact",
  "data": {
    "hitStop": { "timeScale": 0.0, "duration": 0.06 },
    "cameraShake": { "duration": 0.12, "magnitude": 0.3 },
    "particle": "asset/Particle/hit_spark.particle.json",
    "visualEffect": "asset/VisualEffect/hit_flash.vfx.json"
  }
}
```

これは描画リソースではなく、既存Controllerや各Systemへ命令を配送するオーケストレーションデータである。

## Animation Event System

Animation EventはFBXへ直接埋め込むより、モデルまたはClipに対応したサイドカーJSONとして保存する。

```json
{
  "type": "AnimationEvents",
  "formatVersion": 1,
  "model": "asset/Model/player_model.fbx",
  "clips": {
    "Attack01": [
      { "time": 0.12, "name": "EnableHitbox", "payload": "Sword" },
      { "time": 0.18, "name": "PlayEffect", "payload": "asset/GameplayEffect/slash.effect.json" },
      { "time": 0.32, "name": "DisableHitbox", "payload": "Sword" }
    ]
  }
}
```

基本構造:

```cpp
struct AnimationEvent
{
    float time;
    std::string name;
    std::string payload;
};
```

イベントは、前フレーム時刻から現在時刻までに通過した全件を通知する。
低FPSでも飛び越したイベントを失わないこと。

ループ時は範囲を分割する。

```text
通常: previousTime -> currentTime
ループ: previousTime -> duration と 0 -> currentTime
```

初期仕様を次に限定する。

- 正方向再生のみ
- 同一フレームで通過した全イベントを時刻順に発火
- Editor Scrub中はゲームイベントを発火しない
- Pause中は発火しない
- Animation切替直後に過去イベントを誤発火しない
- Transition中にどちらのClipを採用するかを明文化する。初期実装は現在の主Clipのみでよい

イベント名は可能なら登録制にし、Editorでは自由入力よりComboBoxを優先する。
通知はSignal/Event QueueなどでBehaviorへ渡し、Animation Processorがゲーム固有処理を直接呼ばない。

Editor初期機能:

- Clip選択
- Play、Pause、Restart
- 現在時刻のScrub
- Event追加、削除、移動
- Event名とPayload編集
- Preview
- Save、Reload

基本版の難易度は中。逆再生、Transition Blend、複数ループを1フレームで跨ぐ高速再生まで扱うと難易度が上がる。

## エディタ共通部品

専用Editor間で次を共有する。

- Asset Browser / Asset Picker
- ファイルパス正規化
- Save、Save As、Reloadと未保存表示
- Preview用Render Target、Camera、Grid
- Float Curve Editor
- Color / Gradient Editor
- Texture / Model / Material選択UI
- エラー・警告表示

ただし、最初から巨大な汎用Inspector Frameworkを作らない。
Particle Editorなど実際の用途を一つ完成させ、そこで重複が確認できた部品だけ共通化する。

Undo/Redo、サムネイル生成、ドラッグ＆ドロップ、依存関係検索は有用だが、初期の必須要件ではない。

## 推奨ディレクトリ

```text
asset/
  Data/
    Player/
    Enemy/
    Attack/
  Material/
  Particle/
  VisualEffect/
  GameplayEffect/
  AnimationEvent/
  Model/
  Texture/
```

拡張子の例:

```text
*.player.json
*.attack.json
*.mat.json
*.particle.json
*.vfx.json
*.effect.json
*.animation-events.json
```

## 推奨実装順

1. 共通Asset基盤
   - JSON、AssetHeader、相対パス、Cache、Save/Load/Reload、エラー処理
2. ParticleSystem Assetの一貫した往復
   - Editorで編集 → JSON保存 → Reload → ゲーム内再生
3. Material Asset
   - MaterialDesc、JSON、Preview、Reload、MaterialInstance確認
4. DataObjects
   - PlayerやAttackなど実際に必要な型から追加
5. VisualEffect
   - 1 Mesh + 1 MaterialInstance + Curveに限定
6. Animation Event
   - 単一Clip、正方向再生から開始
7. Gameplay Effect
   - HitStop、Camera、Particle、VisualEffectなどの呼び出しを束ねる
8. 必要性が確認できた段階で共通Editor部品を拡張

最初の重要な到達点は次の一本を完成させることである。

> Particleを専用Editorで編集し、`.particle.json`へ保存し、Reloadして、ゲーム画面で同じ結果を再生できる。

これによりAsset基盤、シリアライズ、Preview、Editor操作、実行時反映を一度に検証できる。

## 実装時の判断ルール

- 新機能を追加する前に、保存データ・共有Resource・Instanceのどこに属するか決める
- 生ポインタをJSON相当データへ混ぜない
- 共有Resourceを演出の一時値で変更しない
- C++の再コンパイルなしで調整したい値だけをJSONへ出す
- Game固有の型を無理にEngine汎用機能へ昇格させない
- 専用Editorで十分なものを、汎用ノードエディタへ発展させない
- Reload失敗時は利用中の正常な旧Resourceを維持する
- JSONには必ずデフォルト値と`formatVersion`を持たせる
- 実装済みの挙動を変更するときは、既存アセットとの後方互換または移行方法を用意する
- UIを作る前に、コードからSave/Load/Reloadできることをテストする

## 現行MiEngineから引き継げる土台

この文書作成時点の現行MiEngineには、次の土台がある。

- `ParticleSystemData`にMain、Emission、Shape、SizeOverLifetime、TextureSheetAnimation、Rendererが存在する
- `ParticleSystemAsset`と`ParticleSystemDesc`が存在する
- `ParticleSystemComponent`にDescと粒子の実行時状態が存在する
- `MaterialResource`、`MaterialInstance`、`MaterialRepository`が存在する
- Materialの基本値とCustom Propertyを編集するInspectorが存在する
- Model Animationの再生と時刻更新を行う`AnimationProcessor`が存在する
- HitStop、FOV、Camera Shakeなどを扱う`GameEffectController`が存在する

一方、次は未完成または未導入として扱う。

- 共通Asset Registry / Asset Browser
- 統一されたJSON Serialize / Deserialize基盤
- Material JSONの実読み込み
- Particle AssetのSave / Load / Reload
- Particle専用Editor
- VisualEffect Asset
- Animation Eventの保存、編集、発火

次プロジェクトへコードを移植する場合も、既存クラス名や配置を盲目的に維持せず、この文書の責務分離を優先して再配置してよい。

## 最終ゴール

MiEngineが目指すのは、汎用ゲームエンジンではなく、C++の再コンパイルなしにゲームの見た目・演出・手触りを素早く調整できる軽量な制作環境である。

成功の基準は機能数ではなく、次の編集ループが短く安定していることとする。

```text
Assetを選ぶ
  -> ImGuiで編集する
  -> Previewで確認する
  -> JSONへ保存する
  -> Reloadする
  -> 実ゲームで即座に確認する
```
