#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    /**
        Operations supported directly by the transform system.
     */
    enum class TransformOperation {
        Reverse  = 0b01,
        Flip     = 0b10,
    };

    /**
        Transform instructions that the UI can send.
     */
    enum class TransformInstruction : std::uint8_t {
		Rotate90       = 0,
		Rotate180      = 1,
		Rotate270      = 2,
		FlipHorizontal = 3,
		FlipVertical   = 4,
    };

    /**
        Actual buffer transform state.
     */
    enum class Transform : std::uint8_t {
        Identity  = 0b000, // No transform
        Rotate90  = 0b001, // Rotate 90 degrees clockwise
		Rotate180 = 0b010, // Rotate 180 degrees
		Rotate270 = 0b011, // Rotate 90 degrees counterclockwise
        Mirror    = 0b100, // Flip horizontally
        Mirror90  = 0b101, // Flip horizontally and rotate 90 degrees clockwise
		Mirror180 = 0b110, // Flip horizontally and rotate 180 degrees
		Mirror270 = 0b111, // Flip horizontally and rotate 90 degrees counterclockwise
    };

    Transform operator+(Transform a, TransformInstruction b);
    Transform& operator+=(Transform& a, TransformInstruction b);

    // ------------------------------------------------

    /** 
        Caches transforms of the original buffer, so as to not redo heavy work.
     */
    class TransformCache {
    public:

        // ------------------------------------------------

		// Clears the cache, removing all stored transforms.
        void invalidate();

		// Clears the cache, removing all stored transforms, except Identity.
        void clearExceptIdentity();

        // ------------------------------------------------

        /** Store a transformed buffer in the cache.
        * 
            @param t                the transform that was applied to get this buffer.
			@param buffer           the transformed buffer to store.
         */
        void store(Transform t, const juce::AudioBuffer<float>& buffer);

        /** Get a transformed buffer from the cache.
        
            @param t                the transform to get the buffer for.

			@returns the cached buffer for the given transform. Throws if not found.
         */
        const juce::AudioBuffer<float>& get(Transform t) const;

        /** Check if a transformed buffer is in the cache.
        
            @param t                the transform to check for.

			@returns true if a buffer for the given transform is in the cache, false otherwise.
         */
        bool contains(Transform t) const;

        // ------------------------------------------------

    private:
        std::map<Transform, juce::AudioBuffer<float>> m_Cache{};

        // ------------------------------------------------

    };

    // ------------------------------------------------

}

// ------------------------------------------------

template <>
struct std::formatter<Kaixo::Processing::Transform> : std::formatter<std::string_view> {
    auto format(Kaixo::Processing::Transform t, std::format_context& ctx) const {
        using namespace std::literals;

        std::string_view name = "Unknown"sv;

        switch (t)
        {
        case Kaixo::Processing::Transform::Identity:  name = "Identity";  break;
        case Kaixo::Processing::Transform::Rotate90:  name = "Rotate90";  break;
        case Kaixo::Processing::Transform::Rotate180: name = "Rotate180"; break;
        case Kaixo::Processing::Transform::Rotate270: name = "Rotate270"; break;
        case Kaixo::Processing::Transform::Mirror:    name = "Mirror";    break;
        case Kaixo::Processing::Transform::Mirror90:  name = "Mirror90";  break;
        case Kaixo::Processing::Transform::Mirror180: name = "Mirror180"; break;
        case Kaixo::Processing::Transform::Mirror270: name = "Mirror270"; break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};

template <>
struct std::formatter<Kaixo::Processing::TransformOperation> : std::formatter<std::string_view> {
    auto format(Kaixo::Processing::TransformOperation t, std::format_context& ctx) const {
        using namespace std::literals;
        using namespace Kaixo::Processing;

        std::stringstream result{};

        bool hasFlip    = static_cast<std::uint8_t>(t) & static_cast<std::uint8_t>(Kaixo::Processing::TransformOperation::Flip);
        bool hasReverse = static_cast<std::uint8_t>(t) & static_cast<std::uint8_t>(Kaixo::Processing::TransformOperation::Reverse);

        if (hasReverse)  result << "Reverse | ";
        if (hasFlip)     result << "Flip | ";

        std::string str = result.str();
        if (str.empty()) str = "Identity";
        else str = str.substr(0, str.size() - 3);

        return std::formatter<std::string_view>::format(str, ctx);
    }
};

template <>
struct std::formatter<Kaixo::Processing::TransformInstruction> : std::formatter<std::string_view> {
    auto format(Kaixo::Processing::TransformInstruction t, std::format_context& ctx) const {
        using namespace std::literals;

        std::string_view name = "Unknown"sv;

        switch (t)
        {
        case Kaixo::Processing::TransformInstruction::FlipHorizontal: name = "FlipHorizontal"; break;
        case Kaixo::Processing::TransformInstruction::FlipVertical:   name = "FlipVertical";   break;
        case Kaixo::Processing::TransformInstruction::Rotate90:       name = "Rotate90";       break;
        case Kaixo::Processing::TransformInstruction::Rotate180:      name = "Rotate180";      break;
        case Kaixo::Processing::TransformInstruction::Rotate270:      name = "Rotate270";      break;
        }

        return std::formatter<std::string_view>::format(name, ctx);
    }
};

// ------------------------------------------------
