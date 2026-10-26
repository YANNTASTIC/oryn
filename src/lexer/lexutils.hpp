#pragma once

namespace LexUtils {
    // A-Z, a-z, and underscore (_) are standard identifier starts
    inline constexpr bool is_alpha(char c) noexcept {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
    }

    // 0-9
    inline constexpr bool is_digit(char c) noexcept {
        return c >= '0' && c <= '9';
    }

    // Identifiers can contain digits after the first character
    inline constexpr bool is_alnum(char c) noexcept {
        return is_alpha(c) || is_digit(c);
    }

    // Basic spaces, tabs, and line termination
    inline constexpr bool is_whitespace(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    // Hexadecimal characters for literal notation (0-9, a-f, A-F)
    inline constexpr bool is_xdigit(char c) noexcept {
        return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    // Operators and formatting punctuation found in standard programming syntaxes
    inline constexpr bool is_punc(char c) noexcept {
        return (c >= '!' && c <= '/') ||  // ! " # $ % & ' ( ) * + , - . /
               (c >= ':' && c <= '@') ||  // : ; < = > ? @
               (c >= '[' && c <= '`') ||  // [ \ ] ^ _ `
               (c >= '{' && c <= '~');    // { | } ~
    }
}

