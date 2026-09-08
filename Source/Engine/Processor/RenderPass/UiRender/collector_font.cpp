//===================================================
// collector_font.cpp
// 
// ・TrueTypeフォントを集める
// 
// Author：Miu Kitamura
// Date  ：2025/11/18
//===================================================
#define NOMINMAX
#include "collector_font.h"
#include "Engine/Graphics/font_repository.h"
#include "Engine/Component/text_component.h"

#include "Engine/Device/direct3d.h"

#include "Engine/Core/scene_interface.h"
#include "Engine/Core/game_object.h"

#include "Engine/Graphics/shader_resource.h"
#include "Engine/Component/rect_transform_component.h"
#include "Engine/Component/transform_component.h"

#include "Engine/engine_service_locator.h"

#define SHADER_REPOSITORY EngineServiceLocator::GetShaderRepository()

void CollectorFont::Initialize()
{
    m_fontRepository = EngineServiceLocator::GetFontRepository();
    m_pFontShader = SHADER_REPOSITORY->GetShaderProgramResource("TTFUi");
    if (m_pFontShader) return;
    ShaderProgramResource shader;
    shader.name = "TTFUi";
    shader.baseShader = SHADER_REPOSITORY->GetShaderProgramResource(ShaderBase::Ui);
    shader.overridePixelShader = SHADER_REPOSITORY->GeneratePixelShaderResource("ttf_ui_ps.cso");
    m_pFontShader = SHADER_REPOSITORY->GenerateShaderProgramResource(shader);
}

void CollectorFont::Finalize()
{
    m_fontRepository = nullptr;
    m_pFontShader = nullptr;
}
// シーンからTextComponentを収集して描画バッチを生成
void CollectorFont::CollectDrawBatches2D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch2D>& outBatches)
{
    if (!m_fontRepository) return;
	auto* textPool = pScene->GetComponentPool<TextComponent>();
	auto* rectTransformPool = pScene->GetComponentPool<RectTransformComponent>();
    if (textPool == nullptr || rectTransformPool == nullptr)return;

    auto& textList = textPool->GetList();

	for(TextComponent& t : textList) {
		TextComponent* text = &t;
		RectTransformComponent* rect = rectTransformPool->GetByGameObjectID(text->GetOwner()->GetID());

        // コンポーネントが無効ならスキップ
        if (!rect) continue;
        if (!text->GetEnable()) continue;
		if (!rect->GetOwner()->GetActive()) continue;
		if (!rect->GetEnable()) continue;

        auto* font = m_fontRepository->GetFontResource(text->GetFontPath());
        if (!font) continue;

		// 描画スケール計算
        const float renderScale = static_cast<float>(text->GetFontSize()) / FontResource::BASE_FONT_SIZE;
        const auto textScale = rect->GetScaling();
        const float scaleX = renderScale * textScale.x;
        const float scaleY = renderScale * textScale.y;
        const char8_t* current_char = text->GetText().c_str();

		// 描画コマンドに追加
		UiDrawCommand::DrawBatch2D batch;
		batch.orderInLayer = rect->GetPosition().z;
		batch.texture = font->atlasSRV.Get();
        batch.shaderProgram = m_pFontShader;

		UiDrawCommand::DrawCommand2DInstance instance;
		instance.angleZ = rect->GetRotation().z;
		instance.color = text->GetColor();

		// 各文字の出力
		XMFLOAT3 rectPos = rect->GetPosition();

        XMFLOAT3 pivotPos = { 0.0f, 0.0f, 0.0f };
		{
			// Textを中央揃えにする場合、開始位置を調整
			float xOffset = 0.0f;
			if (text->IsCenter()) {
				while (*current_char != '\0')
				{
					int codepoint = DecodeUtf8(&current_char);
					if (codepoint == 0) break;

					const FontGlyphInfo* glyph = m_fontRepository->GetGlyph(*font, codepoint);
					if (!glyph) continue;

					xOffset += (glyph->x_advance * scaleX) * 0.5f;
				}
			}
			pivotPos.x -= xOffset;
		}

		current_char = text->GetText().c_str();
		while (*current_char != '\0') {
			int codepoint = DecodeUtf8(&current_char);
			if (codepoint == 0) break;

			const FontGlyphInfo* glyph = m_fontRepository->GetGlyph(*font, codepoint);
			if (!glyph) continue;

            // ローカルな頂点データを計算
			float x0 = pivotPos.x + (glyph->x_off * scaleX);
			float y0 = pivotPos.y + (glyph->y_off * scaleY); // ★Y方向を反転
			float x1 = x0 + (glyph->width * scaleX);
			float y1 = y0 + (glyph->height * scaleY); // ★Y方向を反転

			// インスタンス更新
            XMFLOAT2 localPos = { (x0 + x1) / 2.0f, (y0 + y1) / 2.0f };
			{
				// ピボット位置からのオフセットを回転させて加算
				float cosAngle = cosf(instance.angleZ);
				float sinAngle = sinf(instance.angleZ);
				XMFLOAT3 rotatedOffset = {
                    localPos.x* cosAngle - localPos.y * sinAngle,
                    localPos.x* sinAngle + localPos.y * cosAngle,
					0.0f
				};
				instance.position = {
					rectPos.x + rotatedOffset.x,
					rectPos.y + rotatedOffset.y,
                };
			}
			
			instance.size = {
				x1 - x0,
				y1 - y0
            };
			instance.uvRect = {
				glyph->u0, glyph->v0, glyph->u1 - glyph->u0, glyph->v1 - glyph->v0
			};

			// 文字送り (変更なし)
			pivotPos.x += (glyph->x_advance * scaleX);

			// インスタンス追加
			batch.instances.push_back(instance);
		}

		// バッチ追加
		outBatches.push_back(batch);
    }
}

void CollectorFont::CollectDrawBatches3D(IScene* pScene, std::vector<UiDrawCommand::DrawBatch3D>& outBatches)
{
    if (!m_fontRepository) return;
    auto* texts = pScene->GetComponentPool<TextComponent>();
    auto* transforms = pScene->GetComponentPool<TransformComponent>();
    if (!texts || !transforms) return;

    for (auto& text : texts->GetList()) {
        auto* owner = text.GetOwner();
        if (!owner || !owner->GetActive() || !text.GetEnable() || text.GetText().empty()) continue;

        auto* transform = transforms->GetByGameObjectID(owner->GetID());
        if (!transform || !transform->GetEnable()) continue;

        auto* font = m_fontRepository->GetFontResource(text.GetFontPath());
        if (!font || text.GetFontSize() <= 0) continue;
        auto* texture = font->atlasSRV.Get();

        const auto scale = transform->GetScaling();
        const float fontScale = text.GetFontSize() / (FontResource::BASE_FONT_SIZE * FontResource::BASE_FONT_SIZE);
        const float sx = fontScale * scale.x;
        const float sy = fontScale * scale.y;
        float penX = 0.0f;
        if (text.IsCenter()) {
            const char8_t* current = text.GetText().c_str();
            while (*current) {
                const auto* glyph = m_fontRepository->GetGlyph(*font, DecodeUtf8(&current));
                if (glyph) penX -= glyph->x_advance * sx * 0.5f;
            }
        }

        auto& batch = UiDrawCommand::FindOrAddBatch3D(outBatches, texture, m_pFontShader);
        const char8_t* current = text.GetText().c_str();
        while (*current) {
            const auto* glyph = m_fontRepository->GetGlyph(*font, DecodeUtf8(&current));
            if (!glyph) continue;
            UiDrawCommand::DrawCommand3DInstance instance;
            instance.position = transform->GetPosition();
            instance.offset = { penX + (glyph->x_off + glyph->width * 0.5f) * sx,
                (glyph->y_off + glyph->height * 0.5f) * sy };
            instance.scale = { glyph->width * sx, glyph->height * sy, 1.0f };
            instance.color = text.GetColor();
            instance.uvRect = { glyph->u0, glyph->v0, glyph->u1 - glyph->u0, glyph->v1 - glyph->v0 };
            batch.instances.push_back(instance);
            penX += glyph->x_advance * sx;
        }
    }
}

// UTF-8デコード関数
int CollectorFont::DecodeUtf8(const char8_t** text_ptr) 
{
	const unsigned char* s = (const unsigned char*)*text_ptr;
	unsigned int c = *s;
	unsigned int c2, c3, c4;

	if (c < 0x80) { // 1-byte sequence (ASCII)
		*text_ptr += 1;
		return c;
	}
	else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
		if (s[1] != '\0') {
			c2 = s[1];
			if ((c2 & 0xC0) == 0x80) {
				*text_ptr += 2;
				return ((c & 0x1F) << 6) | (c2 & 0x3F);
			}
		}
	}
	else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
		if (s[1] != '\0' && s[2] != '\0') {
			c2 = s[1]; c3 = s[2];
			if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
				*text_ptr += 3;
				return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
			}
		}
	}
	else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
		if (s[1] != '\0' && s[2] != '\0' && s[3] != '\0') {
			c2 = s[1]; c3 = s[2]; c4 = s[3];
			if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
				*text_ptr += 4;
				return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
			}
		}
	}

	// 不正な文字の場合は1バイト進めて置換文字を返す
	*text_ptr += 1;
	return 0xFFFD; // Replacement Character
}
