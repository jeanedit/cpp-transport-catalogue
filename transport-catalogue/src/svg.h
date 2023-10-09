#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <string_view>
#include <optional>

using namespace std::string_view_literals;
namespace svg {

    enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<< (std::ostream& out, const StrokeLineCap line_cap);
    
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& operator<< (std::ostream& out, const StrokeLineJoin line_join);
    
struct Rgb{
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    Rgb(uint8_t r,uint8_t g,uint8_t b):red(r),green(g),blue(b){};
    Rgb() = default;
};
    
struct Rgba{
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
    Rgba(uint8_t r,uint8_t g,uint8_t b,double o):red(r),green(g),blue(b),opacity(o){};
    Rgba() = default;
};
    
using Color = std::variant<std::monostate,std::string,Rgb,Rgba>;
inline const Color NoneColor{"none"};
    
std::ostream& operator<<(std::ostream&out,const Color& color);
    

    
struct ColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        out << NoneColor;
    }
    void operator()(const std::string& str) const {
        out << str;
    }
    void operator()(Rgb rgb) const {
        out << "rgb("sv << rgb.red+0 << ","sv << rgb.green+0 << ","sv<<rgb.blue+0<<")"sv;
    }
    void operator()(Rgba rgba) const {
        out << "rgba("sv << rgba.red+0<< ","sv << rgba.green+0 << ","<<rgba.blue+0<<","sv<<rgba.opacity<<")"sv;
    }
};
    

    
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
* It's an auxiliary structure that stores context for rendering an SVG document with indentation. 
* It contains a reference to the output stream, the current indentation level, and the indentation step when rendering an element.
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) 
    {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) 
    {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

   
/*
* The abstract base class Object serves as a unified way to store specific SVG document tags 
* and implements the "Template Method" pattern for rendering the content of a tag.
 */
      
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
    
 template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeWidth(double width) {
        width_ = width;
        return AsOwner();
    }
    
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }
    
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv<<*fill_color_<<"\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv<<*stroke_color_<<"\""sv;
        }
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv<<*line_cap_<< "\""sv;
        }
        if(line_join_){
            out << " stroke-linejoin=\""sv<<*line_join_<< "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // The static_cast safely converts *this to Owner& if the class Owner is derived from the class PathProps, 
        // or if they have a suitable relationship in the inheritance hierarchy
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};   
    
class ObjectContainer{
    public:
        template<typename Obj>
        void Add(Obj obj){
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    protected:
        ~ObjectContainer() = default;
};
    
class Drawable{
  public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};
    
    
/*
* The Circle class models an <circle> element for displaying a circle in SVG.
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle.
 */
class Circle final : public Object,public PathProps<Circle> {
public:
    Circle() = default;
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = {0.0, 0.0};
    double radius_ = 1.0;
};

/*
* The Polyline class models an <polyline> element for displaying polyline or line segments in SVG.
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline.
 */
class Polyline final:public Object,public PathProps<Polyline> {
public:
    // Adds the next vertex to the polyline.
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * The Text class models an <text> element for displaying text.
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final:public Object,public PathProps<Text> { 
public:
    Text() = default;
    // Sets the coordinates of the anchor point (attributes x and y).
    Text& SetPosition(Point pos);

    // Sets the offset relative to the anchor point (attributes dx and dy).
    Text& SetOffset(Point offset);

    // Sets the font size (attribute font-size).
    Text& SetFontSize(uint32_t size);

    // Sets the font name (attribute font-family).
    Text& SetFontFamily(std::string font_family);

    // Sets the font thickness (attribute font-weight).
    Text& SetFontWeight(std::string font_weight);

    // Sets the text content of the object (displayed inside the <text> tag).
    Text& SetData(const std::string& data);

    // Other data and methods necessary for implementing the <text> element.
    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_ = {0.0, 0.0};
        Point offset_ = {0.0, 0.0};
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
        std::unordered_map<char,std::string> symb_to_shield_ =
        {{'\"',"&quot;"},{'\'',"&apos;"},{'<',"&lt;"},{'>',"&gt;"},{'&',"&amp;"}};
};

class Document:public ObjectContainer{
public:
    /*
     The Add method adds any object that inherits from svg::Object to the SVG document. 
     Here's an example of how to use it:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    // void Add(???);
    Document() = default;

    // The Add method adds an object that is a descendant of svg::Object to the SVG document.
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // The ostream is used to output the SVG representation of the document.
    void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg
