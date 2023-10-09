#include "svg.h"

namespace svg {

using namespace std::literals;
    
std::ostream& operator<<(std::ostream&out,const Color& color){
    std::visit(ColorPrinter{out},color);
    return out;
}
std::ostream& operator<< (std::ostream& out, const StrokeLineCap line_cap){
        switch(line_cap){
            case StrokeLineCap::BUTT:
                out<<"butt";
                break;
            case StrokeLineCap::ROUND:
                out<<"round";
                break;
            case StrokeLineCap::SQUARE:
                out<<"square";
                break;
            }
    return out;
}
    
std::ostream& operator<< (std::ostream& out, const StrokeLineJoin line_join){
    switch(line_join){
            case StrokeLineJoin::ARCS:
                out<<"arcs";
                break;
            case StrokeLineJoin::BEVEL:
                out<<"bevel";
                break;
            case StrokeLineJoin::MITER:
                out<<"miter";
                break;
            case StrokeLineJoin::MITER_CLIP:
                out<<"miter-clip";
                break;
            case StrokeLineJoin::ROUND:
                out<<"round";
                break;   
            }
    return out;
}
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    
    // Delegate the rendering of the tag to our subclasses.
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------
// Adds the next vertex to the polyline.
Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    size_t count = 0;
    for(const Point point:points_){
        out<<point.x<<','<<point.y;
        ++count;
        if(count!=points_.size()){
            out<<' ';
        }
    }
    out<<"\"";
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------
// Sets the coordinates of the anchor point (attributes x and y).
Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

// Sets the offset relative to the anchor point (attributes dx and dy).
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Sets the font size (attribute font-size).
Text& Text::SetFontSize(uint32_t size){
    size_ = size;
    return *this;
}

// Sets the font name (attribute font-family).
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

// Sets the font thickness (attribute font-weight).
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

// Sets the text content of the object (displayed inside the <text> tag).
Text& Text::SetData(const std::string& data){
    data_.reserve(data.size() + 5*symb_to_shield_.size());
    for(const char c:data){
        if(symb_to_shield_.count(c)){
           data_.insert(data_.size(),symb_to_shield_.at(c));
        }
        else{
            data_.push_back(c);
        }
    }
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out<<" x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_<<"\"";
    if(!font_family_.empty()){
        out<< " font-family=\""sv << font_family_ <<"\"";
    }
    if(!font_weight_.empty()){
        out<< " font-weight=\""<<font_weight_<<"\""sv;
    }
    out<<">";
    out << data_;
    out << "</text>"sv;
}
    
// ---------- Document ------------------
// Adds an SVG object, which is a descendant of svg::Object, to the SVG document.
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

// Outputs the SVG representation of the document to an ostream.
void Document::Render(std::ostream& out) const{
    RenderContext ctx(out,2,2);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for(const auto& object:objects_){
        object->Render(ctx);
    }
    out << "</svg>"sv;
}
    
}  // namespace svg
