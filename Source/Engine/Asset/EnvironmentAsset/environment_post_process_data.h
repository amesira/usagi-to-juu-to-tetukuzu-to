//---------------------------------------------------
// File  ：_/Asset/EnvironmentAsset/environment_post_process_data.h
// Date  ：2026/09/04
// Author：Miu Kitamura
// 
// ・ポストプロセス設定のデータ構造を定義するヘッダファイル
//---------------------------------------------------
#pragma once

struct BloomSettings {
    bool enabled = false;
    float intensity = 1.0f;
    float threshold = 1.0f;
};

struct EnvironmentPostProcessData {
    BloomSettings bloom;
};