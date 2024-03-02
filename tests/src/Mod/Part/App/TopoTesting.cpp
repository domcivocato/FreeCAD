#include "gtest/gtest.h"
#include "src/App/InitApplication.h"
#include <Mod/Part/App/TopoShape.h>
#include "Mod/Part/App/TopoShapeMapper.h"
#include <Mod/Part/App/TopoShapeOpCode.h>

#include "PartTestHelpers.h"

#include <BRepAdaptor_CompCurve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFeat_SplitShape.hxx>
#include <BRepOffsetAPI_MakeEvolved.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <gp_Pln.hxx>
#include <ShapeFix_Wireframe.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>

// NOLINTBEGIN(readability-magic-numbers,cppcoreguidelines-avoid-magic-numbers)

using namespace Part;
using namespace PartTestHelpers;

class DomiTest: public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }

    void SetUp() override
    {
        _docName = App::GetApplication().getUniqueDocumentName("test");
        App::GetApplication().newDocument(_docName.c_str(), "testUser");
        _hasher = Base::Reference<App::StringHasher>(new App::StringHasher);
        ASSERT_EQ(_hasher.getRefCount(), 1);
    }

    void TearDown() override
    {
        App::GetApplication().closeDocument(_docName.c_str());
    }


private:
    std::string _docName;
    Data::ElementIDRefs _sid;
    App::StringHasherRef _hasher;
};



/* TEST_F(TopoShapeExpansionTest, DomiTest)
{
    // Arrange
    auto [face1, wire1, edge1, edge2, edge3, egde4] = CreateRectFace();

    std::cout<< topoShape1.makeElementWires()<< std::endl;
    auto wires = topoShape1.getSubShapes(TopAbs_WIRE);
    TopoShape topoShape2 {wires[0], 2L};
    // Act
    auto params = TopoShape::BRepFillingParams();
    TopoShape& result = topoShape1.makeElementFilledFace({topoShape2}, params);
    auto elements = elementMap(result);
    Base::BoundBox3d bb = result.getBoundBox();
    // Assert shape is correct
    EXPECT_TRUE(PartTestHelpers::boxesMatch(bb, Base::BoundBox3d(0.0, -0.6, -0.6, 0, 1.6, 1.6)));
    EXPECT_FLOAT_EQ(getArea(result.getShape()), 1);
    // Assert elementMap is correct
    EXPECT_TRUE(allElementsMatch(result,
                                 {
                                     "Edge1;:G;FFC;:H2:7,E",
                                     "Edge1;:G;FFC;:H2:7,E;:L(Edge2;:G;FFC;:H2:7,E|Edge3;:G;FFC;:"
                                     "H2:7,E|Edge4;:G;FFC;:H2:7,E);FFC;:H2:47,F",
                                     "Edge2;:G;FFC;:H2:7,E",
                                     "Edge3;:G;FFC;:H2:7,E",
                                     "Edge4;:G;FFC;:H2:7,E",
                                     "Vertex1;:G;FFC;:H2:7,V",
                                     "Vertex2;:G;FFC;:H2:7,V",
                                     "Vertex3;:G;FFC;:H2:7,V",
                                     "Vertex4;:G;FFC;:H2:7,V",
                                 }));
}
 */

TEST_F(DomiTest, DomiTest1)
{
    // Arrange
    auto [cube1, cube2] = CreateTwoTopoShapeCubes();
    // We can't use a compound in replaceElementShape, so we'll make a replacement wire and a shell
    auto wire {BRepBuilderAPI_MakeWire(
                   BRepBuilderAPI_MakeEdge(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(1.0, 0.0, 0.0)),
                   BRepBuilderAPI_MakeEdge(gp_Pnt(1.0, 0.0, 0.0), gp_Pnt(1.0, 1.0, 0.0)),
                   BRepBuilderAPI_MakeEdge(gp_Pnt(1.0, 1.0, 0.0), gp_Pnt(0.0, 0.0, 0.0)))
                   .Wire()};
    auto shell = cube1.makeElementShell();
    auto wires = shell.getSubTopoShapes(TopAbs_WIRE);
    // Act

    //TEST
    auto test = wires[0].getElementMapSize();
    auto test2 = wires[0].getElementMap();
    std::cout << test;//.size();
    for (const auto& element : test2) {
        std::cout << "Key: " << element.name << std::endl;
    }


    
    auto test3 = cube1.getElementMap();
    for (const auto& element : test3) {
        std::cout << "Key: " << element.name << std::endl;
    }

    //auto test4 = cube1.getElementMappedNames(test2.front());
    //for (const auto& element : test4) {
    //    std::cout << "Key: " << element.first.~MappedName() << element.second.value() <<std::endl;
    //}

    TopoShape& result = shell.replaceElementShape(shell, {{wires[0], wire}});
    Base::BoundBox3d bb = result.getBoundBox();
    // Assert shape is correct
    EXPECT_TRUE(PartTestHelpers::boxesMatch(bb, Base::BoundBox3d(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    EXPECT_FLOAT_EQ(getArea(result.getShape()), 5);
    EXPECT_EQ(result.countSubElements("Wire"), 6);
    // Assert that we're creating a correct element map
    EXPECT_TRUE(result.getMappedChildElements().empty());
    EXPECT_TRUE(elementsMatch(
        result,
        {
            "Edge1",         "Edge1;:H1,E",   "Edge1;:H2,E",   "Edge1;:H3,E",   "Edge2",
            "Edge2;:H1,E",   "Edge2;:H2,E",   "Edge2;:H3,E",   "Edge3",         "Edge3;:H1,E",
            "Edge3;:H2,E",   "Edge3;:H3,E",   "Edge4;:H1,E",   "Edge4;:H2,E",   "Edge4;:H3,E",
            "Face1;:H2,F",   "Face1;:H3,F",   "Face1;:H4,F",   "Face1;:H5,F",   "Face1;:H6,F",
            "Vertex1",       "Vertex1;:H1,V", "Vertex1;:H2,V", "Vertex2",       "Vertex2;:H1,V",
            "Vertex2;:H2,V", "Vertex3",       "Vertex3;:H1,V", "Vertex3;:H2,V", "Vertex4;:H1,V",
            "Vertex4;:H2,V",
        }));
}