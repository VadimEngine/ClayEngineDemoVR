#include "Font.h"

Font::Font(AAssetManager* assetManager, const std::string& path) {
    // Open the font file from Android assets
    auto fileData = xr::utils::loadAsset(assetManager, path);

    // Initialize the FreeType library
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LOG_E("ERROR::FREETYPE::Could not init FreeType Library");
        return;
    }

    // Create a face object from the memory buffer
    FT_Face face;
    FT_Error error = FT_New_Memory_Face(ft,
                                        reinterpret_cast<const FT_Byte*>(fileData.data()),
                                        static_cast<FT_Long>(fileData.size()),
                                        0,
                                        &face);

    if (error) {
        LOG_E("ERROR::FREETYPE::Failed to load font from memory. Error code: %d", error);
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; ++c) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            LOG_E("ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_LUMINANCE,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_LUMINANCE,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
        };
        mCharacterFrontInfo_.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &mTextVAO_);
    glGenBuffers(1, &mTextVBO_);
    glBindVertexArray(mTextVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, mTextVBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

const Font::Character* Font::getCharInfo(GLchar theChar) const {
    auto it = mCharacterFrontInfo_.find(theChar);

    if (it != mCharacterFrontInfo_.end()) {
        return &(it->second);
    } else {
        return nullptr;
    }
}

unsigned int Font::getVAO() const {
    return mTextVAO_;
}

unsigned int Font::getVBO() const {
    return mTextVBO_;
}