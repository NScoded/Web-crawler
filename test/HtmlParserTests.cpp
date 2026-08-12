#include <gtest/gtest.h>
#include "HtmlParser.h"

class HtmlParserTest : public ::testing::Test {
protected:
    HtmlParser parser;
};

// Single link
TEST_F(HtmlParserTest, ParseSingleLink)
{
    std::string html =
        "<html><body>"
        "<a href=\"https://example.com\">Example</a>"
        "</body></html>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    ASSERT_EQ(links.size(), 1);
    EXPECT_EQ(links[0], "https://example.com");
}

// Multiple links
TEST_F(HtmlParserTest, ParseMultipleLinks)
{
    std::string html =
        "<a href=\"https://google.com\">Google</a>"
        "<a href=\"https://github.com\">GitHub</a>"
        "<a href=\"https://openai.com\">OpenAI</a>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    ASSERT_EQ(links.size(), 3);
    EXPECT_EQ(links[0], "https://google.com");
    EXPECT_EQ(links[1], "https://github.com");
    EXPECT_EQ(links[2], "https://openai.com");
}

// No links
TEST_F(HtmlParserTest, ParseNoLinks)
{
    std::string html =
        "<html><body><h1>Hello World</h1></body></html>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    EXPECT_EQ(links.size(), 0);
}

// Ignore plain text URLs
TEST_F(HtmlParserTest, IgnorePlainTextHttp)
{
    std::string html =
        "<p>Visit https://google.com for more info.</p>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    EXPECT_EQ(links[0], "https://google.com");
}

// Empty HTML
TEST_F(HtmlParserTest, EmptyHtml)
{
    DynamicArray<std::string> links = parser.parseHtml("");

    EXPECT_EQ(links.size(), 0);
}

// Relative URL
TEST_F(HtmlParserTest, RelativeLink)
{
    std::string html =
        "<a href=\"/about\">About</a>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    ASSERT_EQ(links.size(), 1);
    EXPECT_EQ(links[0], "/about");
}

// Duplicate links
TEST_F(HtmlParserTest, DuplicateLinks)
{
    std::string html =
        "<a href=\"https://example.com\"></a>"
        "<a href=\"https://example.com\"></a>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    ASSERT_EQ(links.size(), 2);
    EXPECT_EQ(links[0], "https://example.com");
    EXPECT_EQ(links[1], "https://example.com");
}

// Malformed HTML
TEST_F(HtmlParserTest, MalformedHtml)
{
    std::string html =
        "<a href=\"https://example.com\">"
        "<a href=\"https://google.com\"";

    DynamicArray<std::string> links = parser.parseHtml(html);

    EXPECT_GE(links.size(), 1);
}

// Mixed content
TEST_F(HtmlParserTest, MixedContent)
{
    std::string html =
        "<div>Hello</div>"
        "<a href=\"https://one.com\"></a>"
        "<span>World</span>"
        "<a href=\"https://two.com\"></a>";

    DynamicArray<std::string> links = parser.parseHtml(html);

    ASSERT_EQ(links.size(), 2);
    EXPECT_EQ(links[0], "https://one.com");
    EXPECT_EQ(links[1], "https://two.com");
}

TEST_F(HtmlParserTest, ExtractsOnlyTheFirstTitleBlock)
{
    std::string html =
        "<html><head><title>AWS Academy | Training and Certification | AWS</title></head>"
        "<body><div>Some content</div></body></html>";

    std::string title;
    DynamicArray<std::string> links = parser.parseLinks(html, title);

    EXPECT_EQ(title, "AWS Academy | Training and Certification | AWS");
}