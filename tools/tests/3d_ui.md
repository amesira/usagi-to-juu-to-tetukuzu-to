# 3D UI

- `ImageComponent` / `SliderComponent` / `TextComponent` と `TransformComponent` を同じGameObjectに付けると3D UIとして描画する。
- `RectTransformComponent` がある場合は2D UIを優先する。3D UIを作るときはRectTransformを付けない（既存のUiFactoryは2D用）。
- Transformの位置をワールド座標として使用する。画像とスライダーの幅・高さはScalingのX・Yで指定する。
- TextのFontSizeは64を基準に約1ワールド単位。ScalingのX・Yで文字サイズと文字間隔を一緒に調整する。中央揃えは既存と同じく横方向のみ。
- 全てカメラの向きに正対するビルボード。Transformの回転とImageComponentのWorldSpaceTypeは3D UIの向きに使用しない。
- 文字の配置オフセットとスライダーの塗りつぶし位置はビルボード面内で変換する。
- `enable3D` と `enableUI` が有効なRenderViewで、ポストエフェクトより前に描画する。
- シーンに対して深度テストを行うが深度を書き込まない。3D UI間の距離ソートは行わず、同一テクスチャ・同一シェーダーのデータをまとめる。
- 2D/3DともGPUバッファ容量を超えた分は2048インスタンスずつに分割して描画する。

## 検証

リポジトリルートで `tools\tests\run_ui_draw_command_tests.cmd` を実行する（Visual Studio 2022 Enterprise使用）。
リソース別バッチ統合、4097個のデータ保持、複数のカメラ姿勢でのビルボード配置、スライダーの左端一致を確認するCPUテスト。
実機での描画確認は別途必要。