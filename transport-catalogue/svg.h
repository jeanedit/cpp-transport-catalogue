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
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
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
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
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
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
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
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
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
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final:public Object,public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final:public Object,public PathProps<Text> { 
public:
    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(const std::string& data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
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
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    // void Add(???);
    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg