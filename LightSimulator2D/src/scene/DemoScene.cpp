#include "DemoScene.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "scene/components/PlayerControllerComponent.h"
#include "scene/components/FollowMouseComponent.h"

static void Scene1(Scene* scene, PlayerControllerComponent* controller);
static void Scene2(Scene* scene, PlayerControllerComponent* controller);
static void Scene3(Scene* scene, PlayerControllerComponent* controller);
static void Scene4(Scene* scene, PlayerControllerComponent* controller);

PlayerControllerComponent* Demo::SetupDemoScene(Scene* scene, unsigned index)
{
    PlayerControllerComponent* controller = new PlayerControllerComponent(scene->GetCamera());

    switch (index)
    {
    case 1: Scene1(scene, controller); break;
    case 2: Scene2(scene, controller); break;
    case 3: Scene3(scene, controller); break;
    case 4: Scene4(scene, controller); break;
    default: Scene2(scene, controller); break;
    }

    return controller;
}

// Between -2.0 and 2.0
static float RandFloat(float min, float max)
{
    return (float)rand() / RAND_MAX * (max-min) + min;
}

static Vec2 RandPos(float min, float max)
{
    Vec2 randPos;
    randPos.x = RandFloat(min, max);
    randPos.y = RandFloat(min, max);
    return randPos;
}

static void Scene1(Scene* scene, PlayerControllerComponent* controller)
{
    scene->GetAmbientLight() = 0.2f * VEC3_ONE;

    // Entities
    Entity* bg = new Entity{ "res/bg.png" , "res/animals/elephant_normal.jpg" };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 5.0f;

    Entity* e1 = new Entity{ "res/animals/elephant.png", "res/animals/elephant_normal3.jpg" };
    e1->GetDrawFlags().occluder = true;
    e1->GetDrawFlags().foreground = true;
    e1->GetOcclusionProperties().shape = OccluderShape::Mesh;
    e1->GetOcclusionProperties().meshLod = 2;
    e1->AddComponent(controller);

    Entity* e2 = new Entity{ "res/animals/hippo.png" };
    e2->GetDrawFlags().emitter = true;
    e2->GetEmissionProperties().color = Vec3(0.0, 0.0, 1.0);
    e2->GetEmissionProperties().radius = 0.05f;

    Entity* e3 = new Entity{ "res/animals/giraffe.png" };
    e3->AddComponent(new FollowMouseComponent());
    e3->GetDrawFlags().emitter = true;
    e3->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    e3->GetEmissionProperties().radius = 0.1f;

    Entity* e4 = new Entity{ "res/animals/elephant.png" , "res/animals/elephant_normal.jpg" };

    scene->AddEntity(bg);
    scene->AddEntity(e1);
    scene->AddEntity(e2);
    scene->AddEntity(e3);
    scene->AddEntity(e4);

    // Instances

    bg->Instance();

    EntityInstance* elephant = e1->Instance();
    elephant->ApplyRotation(3.1415f / 4.0f);

    EntityInstance* hippo = e2->Instance();
    hippo->ApplyScale(0.2f);
    hippo->SetPosition(Vec2(-0.3, 0.5));

    EntityInstance* giraffe = e3->Instance();
    giraffe->ApplyScale(0.3f);

    EntityInstance* elephant2 = e4->Instance();
    elephant2->ApplyScale(0.8f);
    elephant2->SetPosition(Vec2(0.8, 0.8));
}

static void Scene2(Scene* scene, PlayerControllerComponent* controller)
{
    // Entities
    const std::string res_path = "res/scene2/";

    scene->GetAmbientLight() = 0.05f * Vec3(0.0, 1.0, 0.0);

#define P(X) res_path + X

    Entity* bg = new Entity{P("bg_diffuse.png"), P("bg_normal.png") };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 2.0f;

    Entity* emitter = new Entity{ P("lantern_yellow.png") };
    emitter->AddComponent(new FollowMouseComponent());
    emitter->AddComponent(controller);
    emitter->GetDrawFlags().emitter = true;
    emitter->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    emitter->GetEmissionProperties().radius = 0.15f;

    Entity* torchEmitter = new Entity{ P("lantern.png") };
    torchEmitter->GetDrawFlags().emitter = true;
    torchEmitter->GetEmissionProperties().color = Vec3(0.92, 0.51, 0.2);
    torchEmitter->GetEmissionProperties().radius = 0.15f;

    Entity* tree1_up = new Entity{ P("tree1/tree1_up.png") };
    tree1_up->GetDrawFlags().foreground = true;

    Entity* tree1_down = new Entity{P("tree1/tree1_down.png")};
    tree1_down->GetDrawFlags().occluder = true;
    tree1_down->GetOcclusionProperties().shape = OccluderShape::Rect;

    Entity* tree2_up = new Entity{ P("tree2/tree2_up.png") };
    tree2_up->GetDrawFlags().foreground = true;

    Entity* tree2_down = new Entity{ P("tree2/tree2_down.png") };
    tree2_down->GetDrawFlags().occluder = true;
    tree2_down->GetOcclusionProperties().shape = OccluderShape::Rect;

    Entity* bush = new Entity{ P("bush_diffuse.png") };
    
    Entity* fence = new Entity{ P("fence.png") };
    fence->GetDrawFlags().foreground = true;

    Entity* fenceOccluder = new Entity{ P("fence.png") };
    fenceOccluder->GetDrawFlags().occluder = true;
    fenceOccluder->GetOcclusionProperties().shape = OccluderShape::Rect;

    Entity* house = new Entity{ P("house/house_diffuse.png"),P("house/house_normal.png") };
    house->GetDrawFlags().occluder = true;
    house->GetOcclusionProperties().meshLod = 2;
    house->GetOcclusionProperties().shape = OccluderShape::Mesh;

#undef P

    scene->AddEntity(bg);
    scene->AddEntity(emitter);
    scene->AddEntity(torchEmitter);
    scene->AddEntity(tree1_up);
    scene->AddEntity(tree1_down);
    scene->AddEntity(tree2_up);
    scene->AddEntity(tree2_down);
    scene->AddEntity(bush);
    //scene->AddEntity(house);
    scene->AddEntity(fence);
    scene->AddEntity(fenceOccluder);

    // Instances
    const Vec2 tree1Offset = Vec2(0.01, -0.2);
    const Vec2 tree2Offset = Vec2(0.0, -0.27);
    const Vec2 torchOffest = Vec2(-0.27, 0.0);
    const unsigned numBushes = 100;
    const Vec2 numTrees = VEC2_ONE * 20.0f;
    const Vec2 treeArea = numTrees / 1.6f;

    bg->Instance();
    emitter->Instance()->ApplyScale(0.01f);
    //house->Instance()->ApplyScale(3.0f);

    for (unsigned i = 0; i < numBushes; i++)
    {
        const Vec2 tah = treeArea / (VEC2_ONE * 2.0f);
        Vec2 pos = Vec2(RandFloat(-tah.x, tah.x), RandFloat(-tah.y, tah.y));
        bush->Instance()->SetPosition(pos);
    }

    for (unsigned i = 0; i < (unsigned) numTrees.x; i++)
    {
        for (unsigned j = 0; j < (unsigned)numTrees.y; j++)
        {
            const Vec2 nth = numTrees / 2.0f; // Num trees half
            const Vec2 gapSize = VEC2_ONE * 3.0f;

            if ((i < nth.x + gapSize.x && i > nth.x - gapSize.x) && (j < nth.y + gapSize.y && j > nth.y - gapSize.y)) continue;

            const float randVar = 0.1f;
            const Vec2 treeStep = treeArea / numTrees;
            Vec2 treePos = Vec2(i, j) * treeStep - treeArea/2.0f;
            treePos += Vec2(RandFloat(-randVar, randVar), RandFloat(-randVar, randVar));
            float treeDecider = RandFloat(0.0f, 1.0f);

            if (treeDecider > 0.3f)
            {
                tree1_up->Instance()->SetPosition(treePos);
                tree1_down->Instance()->SetPosition(treePos + tree1Offset);
            }
            else
            {
                tree2_up->Instance()->SetPosition(treePos);
                tree2_down->Instance()->SetPosition(treePos + tree2Offset);
            }

            if (i % 3 == 0 && j % 3 == 2)
            {
                EntityInstance* ei = torchEmitter->Instance();
                ei->SetPosition(treePos + torchOffest);
                ei->ApplyScale(0.03f);
            }
        
        }
    }

    constexpr float PI = 3.1415f;
    constexpr unsigned numFences = 10;
    constexpr Vec2 fenceSize = Vec2(0.15f,0.2f);
    constexpr Vec2 halfSide = fenceSize * (float)numFences / 2.0f;
    constexpr Vec2 startPositions[] = { Vec2(-1.0f,-1.0f), Vec2(-1.0f,1.0f), Vec2(-1.0f,-1.0f), Vec2(1.0f,-1.0f) };
    constexpr Vec2 increments[] = { Vec2(1.0f,0.0f), Vec2(1.0f,0.0f), Vec2(0.0f,1.0f), Vec2(0.0f,1.0f)};
    constexpr float rotations[] = { 0.0f,0.0f,PI / 2.0f,PI / 2.0f };
    constexpr Vec2 scales[] = {VEC2_ONE, VEC2_ONE, Vec2(0.8,1.0), Vec2(0.8,1.0) };
    for (unsigned side = 0; side < 4; side++)
    {
        for (unsigned i = 1; i < numFences; i++)
        {
            Vec2 fencePos = startPositions[side]*halfSide + (float) i * increments[side] * fenceSize;
            EntityInstance* ei = fence->Instance();
            ei->SetPosition(fencePos);
            ei->ApplyRotation(rotations[side]);
            ei->ApplyScale(scales[side]);

            Vec2 occluderOffset = side >= 2 ? Vec2(0.04,0.08) : Vec2(0.04, 0.04);
            EntityInstance* o1 = fenceOccluder->Instance();
            o1->SetPosition(fencePos + occluderOffset * scales[side]);
            o1->ApplyRotation(rotations[side]);
            o1->ApplyScale(0.1f);

            EntityInstance* o2 = fenceOccluder->Instance();
            o2->SetPosition(fencePos + occluderOffset * Vec2(1.0f,-1.0f) * scales[side]);
            o2->ApplyScale(0.1f);
            o2->ApplyRotation(rotations[side]);
        }
    }
}

static void Scene3(Scene* scene, PlayerControllerComponent* controller)
{
    const std::string res_path = "res/demo/";

    scene->GetAmbientLight() = 0.2f * Vec3(0.2, 0.8, 1.0);

#define P(X) res_path + X
    Entity* bg = new Entity{ P("bg.png") , P("bg_normal2.jpg") };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 8.f;

    Entity* krug = new Entity{ P("krug.png") };
    krug->AddComponent(new FollowMouseComponent());
    krug->AddComponent(controller);
    krug->GetDrawFlags().emitter = true;
    krug->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    krug->GetEmissionProperties().radius = 0.15f;

    Entity* kvadrat = new Entity{ P("kvadrat.png") , P("bg_normal3.jpg") };
    kvadrat->GetDrawFlags().occluder = true;

    Entity* pravugaonik = new Entity(P("pravugaonik.png"));
    pravugaonik->GetDrawFlags().occluder = true;

    Entity* oblik1 = new Entity(P("oblik.png"));
    oblik1->GetDrawFlags().occluder = true;
    oblik1->GetOcclusionProperties().shape = OccluderShape::Mesh;
    oblik1->GetOcclusionProperties().meshLod = 1;

    Entity* oblik2 = new Entity(P("oblik2.png"));
    oblik2->GetDrawFlags().occluder = true;
    oblik2->GetOcclusionProperties().shape = OccluderShape::Mesh;
    oblik2->GetOcclusionProperties().meshLod = 1;

#undef P

    scene->AddEntity(bg);
    scene->AddEntity(krug);
    scene->AddEntity(kvadrat);
    scene->AddEntity(pravugaonik);
    scene->AddEntity(oblik1);
    scene->AddEntity(oblik2);

    bg->Instance();
    krug->Instance();
    kvadrat->Instance()->SetPosition({ 0.4,0.4 });
    pravugaonik->Instance()->SetPosition({ -0.6, 0.2 });
    oblik1->Instance()->SetPosition({ -0.6,-0.7 });
    oblik2->Instance()->SetPosition({ 0.3,-0.5 });
}

static void Scene4(Scene* scene, PlayerControllerComponent* controller)
{
    const std::string res_path = "res/demo/";

    scene->GetAmbientLight() = 0.2f * Vec3(0.2, 0.8, 1.0);

#define P(X) res_path + X
    Entity* bg = new Entity{ P("bg.png") };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 8.f;

    Entity* kvadrat = new Entity{ P("kvadrat.png") };
    kvadrat->GetDrawFlags().occluder = true;

    Entity* pravugaonik = new Entity(P("pravugaonik.png"));
    pravugaonik->GetDrawFlags().occluder = true;

    Entity* krug = new Entity{ P("krug.png") };
    krug->AddComponent(new FollowMouseComponent());
    krug->AddComponent(controller);
    krug->GetDrawFlags().emitter = true;
    krug->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    krug->GetEmissionProperties().radius = 0.15f;

#undef P

    scene->AddEntity(bg);
    scene->AddEntity(kvadrat);
    scene->AddEntity(pravugaonik);
    scene->AddEntity(krug);

    krug->Instance();
    bg->Instance();
    kvadrat->Instance()->SetPosition({ 0.4,0.4 });
    pravugaonik->Instance()->SetPosition({ -0.6, 0.2 });
}