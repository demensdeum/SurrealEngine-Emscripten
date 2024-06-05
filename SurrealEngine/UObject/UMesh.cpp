
#include "Precomp.h"
#include "UMesh.h"
#include "UTexture.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include <iostream>

void UPrimitive::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	BoundingBox.min.x = stream->ReadFloat();
	BoundingBox.min.y = stream->ReadFloat();
	BoundingBox.min.z = stream->ReadFloat();
	BoundingBox.max.x = stream->ReadFloat();
	BoundingBox.max.y = stream->ReadFloat();
	BoundingBox.max.z = stream->ReadFloat();
	BoundingBox.IsValid = stream->ReadUInt8() != 0;

	BoundingSphere.x = stream->ReadFloat();
	BoundingSphere.y = stream->ReadFloat();
	BoundingSphere.z = stream->ReadFloat();
	BoundingSphere.w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;
}

/////////////////////////////////////////////////////////////////////////////

void UMesh::Load(ObjectStream* stream)
{
	UPrimitive::Load(stream);

	uint32_t VertsSkipOffset = 0;
	if (stream->GetVersion() > 61) VertsSkipOffset = stream->ReadUInt32();
	int NumVerts = stream->ReadIndex();

	// XXX: Something to consider if we decide to load DeusEx packages
	//      without necessarily running DeusEx, we will need some kind
	//      of indicator as to what mesh type we're looking at.
	//      Maybe just some optional metadata in the package that describes 
	//			which type of mesh this is?
std::cout << "GP13" << std::endl;
	if (stream->GetPackage()->GetPackageManager()->IsDeusEx())
	{
		for (int i = 0; i < NumVerts; i++)
		{
			struct DeusExVertex
			{
				int16_t x, y, z, padding;
			} packedvertex;
			stream->ReadBytes(&packedvertex, sizeof(DeusExVertex));
			vec3 vertex = { (float)packedvertex.x, (float)packedvertex.y, (float)packedvertex.z };
			Verts.push_back(vertex);
		}
	}
	else
	{
		for (int i = 0; i < NumVerts; i++)
		{
			int32_t packedvertex = stream->ReadInt32();
			vec3 vertex = { (float)((packedvertex << 21) >> 21), (float)((packedvertex << 10) >> 21), (float)(packedvertex >> 22) };
			Verts.push_back(vertex);
		}
	}
	if (stream->GetVersion() > 61 && stream->Tell() != VertsSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	uint32_t TrisSkipOffset = 0;
	if (stream->GetVersion() > 61) TrisSkipOffset = stream->ReadUInt32();
	int NumTris = stream->ReadIndex();
	for (int i = 0; i < NumTris; i++)
	{
		MeshTri tri;
		tri.Indices[0] = stream->ReadUInt16();
		tri.Indices[1] = stream->ReadUInt16();
		tri.Indices[2] = stream->ReadUInt16();
		tri.UV[0].x = stream->ReadUInt8();
		tri.UV[0].y = stream->ReadUInt8();
		tri.UV[1].x = stream->ReadUInt8();
		tri.UV[1].y = stream->ReadUInt8();
		tri.UV[2].x = stream->ReadUInt8();
		tri.UV[2].y = stream->ReadUInt8();
		tri.PolyFlags = stream->ReadUInt32();
		tri.TextureIndex = stream->ReadInt32();
		Tris.push_back(tri);
	}
	if (stream->GetVersion() > 61 && stream->Tell() != TrisSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	int NumAnimSeq = stream->ReadIndex();
	for (int i = 0; i < NumAnimSeq; i++)
	{
		MeshAnimSeq seq;
		seq.Name = stream->ReadName();
		seq.Group = stream->ReadName();
		seq.StartFrame = stream->ReadInt32();
		seq.NumFrames = stream->ReadInt32();
		int NumNotifys = stream->ReadIndex();
		for (int j = 0; j < NumNotifys; j++)
		{
			MeshAnimNotify notify;
			notify.Time = stream->ReadFloat();
			notify.Function = stream->ReadName();
			seq.Notifys.push_back(notify);
		}
		seq.Rate = stream->ReadFloat();
		std::stable_sort(seq.Notifys.begin(), seq.Notifys.end(), [](auto& a, auto& b) { return a.Time < b.Time; });
		AnimSeqs.push_back(seq);
	}

	uint32_t ConnectsSkipOffset = 0;
	if (stream->GetVersion() > 61) ConnectsSkipOffset = stream->ReadUInt32();
	int NumConnects = stream->ReadIndex();
	for (int i = 0; i < NumConnects; i++)
	{
		MeshVertConnect connect;
		connect.NumVertTriangles = stream->ReadInt32();
		connect.TriangleListOffset = stream->ReadUInt32();
		Connects.push_back(connect);
	}
	if (stream->GetVersion() > 61 && stream->Tell() != ConnectsSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	// Unknown bbox and sphere?
	BBox bbox;
	bbox.min.x = stream->ReadFloat();
	bbox.min.y = stream->ReadFloat();
	bbox.min.z = stream->ReadFloat();
	bbox.max.x = stream->ReadFloat();
	bbox.max.y = stream->ReadFloat();
	bbox.max.z = stream->ReadFloat();
	bbox.IsValid = stream->ReadUInt8() != 0;
	float x = stream->ReadFloat();
	float y = stream->ReadFloat();
	float z = stream->ReadFloat();
	float w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;

	uint32_t VertLinksSkipOffset = 0;
	if (stream->GetVersion() > 61) VertLinksSkipOffset = stream->ReadUInt32();
	int NumVertLinks = stream->ReadIndex();
	for (int i = 0; i < NumVertLinks; i++)
	{
		VertLinks.push_back(stream->ReadInt32());
	}
	if (stream->GetVersion() > 61 && stream->Tell() != VertLinksSkipOffset)
		Exception::Throw("Unexpected lazy array size");

	int NumTextures = stream->ReadIndex();
	for (int i = 0; i < NumTextures; i++)
		Textures.push_back(stream->ReadObject<UTexture>());

	int NumBoundingBoxes = stream->ReadIndex();
	for (int i = 0; i < NumBoundingBoxes; i++)
	{
		BBox bbox;
		bbox.min.x = stream->ReadFloat();
		bbox.min.y = stream->ReadFloat();
		bbox.min.z = stream->ReadFloat();
		bbox.max.x = stream->ReadFloat();
		bbox.max.y = stream->ReadFloat();
		bbox.max.z = stream->ReadFloat();
		bbox.IsValid = stream->ReadUInt8() == 1;
		BoundingBoxes.push_back(bbox);
	}

	int NumBoundingSpheres = stream->ReadIndex();
	for (int i = 0; i < NumBoundingSpheres; i++)
	{
		float x = stream->ReadFloat();
		float y = stream->ReadFloat();
		float z = stream->ReadFloat();
		float w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;
		BoundingSpheres.push_back({ x, y, z, w });
	}

	FrameVerts = stream->ReadInt32();
	AnimFrames = stream->ReadInt32();

	AndFlags = stream->ReadUInt32();
	OrFlags = stream->ReadUInt32();

	Scale.x = stream->ReadFloat();
	Scale.y = stream->ReadFloat();
	Scale.z = stream->ReadFloat();

	Origin.x = stream->ReadFloat();
	Origin.y = stream->ReadFloat();
	Origin.z = stream->ReadFloat();

	RotOrigin.Pitch = stream->ReadInt32();
	RotOrigin.Yaw = stream->ReadInt32();
	RotOrigin.Roll = stream->ReadInt32();

	CurPoly = stream->ReadUInt32();
	CurVertex = stream->ReadUInt32();

	if (stream->GetVersion() == 65)
	{
		TextureLOD.push_back(stream->ReadFloat());
	}
	else if (stream->GetVersion() >= 66)
	{
		int NumTextureLOD = stream->ReadIndex();
		for (int i = 0; i < NumTextureLOD; i++)
		{
			TextureLOD.push_back(stream->ReadFloat());
		}
	}

	meshToObject = Coords::Rotation(RotOrigin).ToMatrix() * mat4::scale(Scale) * mat4::translate(-Origin);

	// Build smoothed normals
	// 
	// To do: build normals from mesh faces (maybe using the Connects array?)
}

/////////////////////////////////////////////////////////////////////////////

void ULodMesh::Load(ObjectStream* stream)
{
	UMesh::Load(stream);

	int NumCollapsePointThus = stream->ReadIndex();
	for (int i = 0; i < NumCollapsePointThus; i++)
		CollapsePointThus.push_back(stream->ReadUInt16());

	int NumFaceLevel = stream->ReadIndex();
	for (int i = 0; i < NumFaceLevel; i++)
		FaceLevel.push_back(stream->ReadUInt16());

	int NumFaces = stream->ReadIndex();
	for (int i = 0; i < NumFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = stream->ReadUInt16();
		face.Indices[1] = stream->ReadUInt16();
		face.Indices[2] = stream->ReadUInt16();
		face.MaterialIndex = stream->ReadUInt16();
		Faces.push_back(face);
	}

	int NumCollapseWedgeThus = stream->ReadIndex();
	for (int i = 0; i < NumCollapseWedgeThus; i++)
		CollapseWedgeThus.push_back(stream->ReadUInt16());

	int NumWedges = stream->ReadIndex();
	for (int i = 0; i < NumWedges; i++)
	{
		MeshWedge wedge;
		wedge.Vertex = stream->ReadUInt16();
		wedge.U = stream->ReadUInt8();
		wedge.V = stream->ReadUInt8();
		Wedges.push_back(wedge);
	}

	int NumMaterials = stream->ReadIndex();
	for (int i = 0; i < NumMaterials; i++)
	{
		MeshMaterial material;
		material.PolyFlags = stream->ReadUInt32();
		material.TextureIndex = stream->ReadInt32();
		Materials.push_back(material);
	}

	int NumSpecialFaces = stream->ReadIndex();
	for (int i = 0; i < NumSpecialFaces; i++)
	{
		MeshFace face;
		face.Indices[0] = stream->ReadUInt16();
		face.Indices[1] = stream->ReadUInt16();
		face.Indices[2] = stream->ReadUInt16();
		face.MaterialIndex = stream->ReadUInt16();
		SpecialFaces.push_back(face);
	}

	ModelVerts = stream->ReadUInt32();
	SpecialVerts = stream->ReadUInt32();
	MeshScaleMax = stream->ReadFloat();
	LODHysteresis = stream->ReadFloat();
	LODStrength = stream->ReadFloat();
	LODMinVerts = stream->ReadUInt32();
	LODMorph = stream->ReadFloat();
	LODZDisplace = stream->ReadFloat();

	int NumReMapAnimVerts = stream->ReadIndex();
	for (int i = 0; i < NumReMapAnimVerts; i++)
	{
		ReMapAnimVerts.push_back(stream->ReadUInt16());
	}

	OldFrameVerts = stream->ReadUInt32();

	// Build smoothed normals
	Normals.clear();
	Normals.resize(Verts.size(), vec3(0.0f));
	for (int frame = 0; frame < AnimFrames; frame++)
	{
		for (const MeshFace& face : SpecialFaces)
		{
			int v0 = Wedges[face.Indices[0]].Vertex + frame * FrameVerts;
			int v1 = Wedges[face.Indices[1]].Vertex + frame * FrameVerts;
			int v2 = Wedges[face.Indices[2]].Vertex + frame * FrameVerts;
			vec3 n = normalize(cross(Verts[v1] - Verts[v0], Verts[v2] - Verts[v0]));
			Normals[v0] += n;
			Normals[v1] += n;
			Normals[v2] += n;
		}
		for (const MeshFace& face : Faces)
		{
			int v0 = Wedges[face.Indices[0]].Vertex + SpecialVerts + frame * FrameVerts;
			int v1 = Wedges[face.Indices[1]].Vertex + SpecialVerts + frame * FrameVerts;
			int v2 = Wedges[face.Indices[2]].Vertex + SpecialVerts + frame * FrameVerts;
			vec3 n = normalize(cross(Verts[v1] - Verts[v0], Verts[v2] - Verts[v0]));
			Normals[v0] += n;
			Normals[v1] += n;
			Normals[v2] += n;
		}
	}
	for (vec3& n : Normals)
	{
		n = normalize(n);
	}
}

/////////////////////////////////////////////////////////////////////////////

void USkeletalMesh::Load(ObjectStream* stream)
{
	ULodMesh::Load(stream);

	int NumExtWedges = stream->ReadIndex();
	for (int i = 0; i < NumExtWedges; i++)
	{
		ExtMeshWedge wedge;
		wedge.Vertex = stream->ReadUInt16();
		wedge.Flags = stream->ReadUInt16();
		wedge.U = stream->ReadFloat();
		wedge.V = stream->ReadFloat();
		ExtWedges.push_back(wedge);
	}

	int NumPoints = stream->ReadIndex();
	for (int i = 0; i < NumPoints; i++)
	{
		vec3 p;
		p.x = stream->ReadFloat();
		p.y = stream->ReadFloat();
		p.z = stream->ReadFloat();
		Points.push_back(p);
	}

	int NumRefSkeletonBones = stream->ReadIndex();
	for (int i = 0; i < NumRefSkeletonBones; i++)
	{
		RefSkeletonBone bone;
		bone.Name = stream->ReadName();
		bone.Flags = stream->ReadUInt32();
		bone.Orientation.x = stream->ReadFloat();
		bone.Orientation.y = stream->ReadFloat();
		bone.Orientation.z = stream->ReadFloat();
		bone.Orientation.w = stream->ReadFloat();
		bone.Position.x = stream->ReadFloat();
		bone.Position.y = stream->ReadFloat();
		bone.Position.z = stream->ReadFloat();
		bone.Length = stream->ReadFloat();
		bone.Size.x = stream->ReadFloat();
		bone.Size.y = stream->ReadFloat();
		bone.Size.z = stream->ReadFloat();
		bone.NumChildren = stream->ReadUInt32();
		bone.ParentIndex = stream->ReadUInt32();
		RefSkeleton.push_back(bone);
	}

	int NumBoneWeightIndices = stream->ReadIndex();
	for (int i = 0; i < NumBoneWeightIndices; i++)
	{
		BoneWeightIndex index;
		index.WeightIndex = stream->ReadUInt16();
		index.Number = stream->ReadUInt16();
		index.DetailA = stream->ReadUInt16();
		index.DetailB = stream->ReadUInt16();
		BoneWeightIndices.push_back(index);
	}

	int NumBoneWeights = stream->ReadIndex();
	for (int i = 0; i < NumBoneWeights; i++)
	{
		BoneWeight weight;
		weight.PointIndex = stream->ReadUInt16();
		weight.BoneWeight = stream->ReadUInt16();
		BoneWeights.push_back(weight);
	}

	int NumLocalPoints = stream->ReadIndex();
	for (int i = 0; i < NumLocalPoints; i++)
	{
		vec3 p;
		p.x = stream->ReadFloat();
		p.y = stream->ReadFloat();
		p.z = stream->ReadFloat();
		LocalPoints.push_back(p);
	}

	SkeletalDepth = stream->ReadUInt32();
	DefaultAnimation = stream->ReadObject<UAnimation>();
	WeaponBoneIndex = stream->ReadUInt32();

	WeaponAdjust.Origin.x = stream->ReadFloat();
	WeaponAdjust.Origin.y = stream->ReadFloat();
	WeaponAdjust.Origin.y = stream->ReadFloat();
	WeaponAdjust.XAxis.x = stream->ReadFloat();
	WeaponAdjust.XAxis.y = stream->ReadFloat();
	WeaponAdjust.XAxis.y = stream->ReadFloat();
	WeaponAdjust.YAxis.x = stream->ReadFloat();
	WeaponAdjust.YAxis.y = stream->ReadFloat();
	WeaponAdjust.YAxis.y = stream->ReadFloat();
	WeaponAdjust.ZAxis.x = stream->ReadFloat();
	WeaponAdjust.ZAxis.y = stream->ReadFloat();
	WeaponAdjust.ZAxis.y = stream->ReadFloat();
}

/////////////////////////////////////////////////////////////////////////////

void UAnimation::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	int NumRefBones = stream->ReadIndex();
	for (int i = 0; i < NumRefBones; i++)
	{
		RefBone refbone;
		refbone.Name = stream->ReadName();
		refbone.Flags = stream->ReadUInt32();
		refbone.ParentIndex = stream->ReadUInt32();
		RefBones.push_back(refbone);
	}

	int NumMoves = stream->ReadIndex();
	for (int i = 0; i < NumMoves; i++)
	{
		AnimMove move;
		move.RootSpeed3D.x = stream->ReadFloat();
		move.RootSpeed3D.y = stream->ReadFloat();
		move.RootSpeed3D.z = stream->ReadFloat();
		move.TrackTime = stream->ReadFloat();
		move.StartBone = stream->ReadUInt32();
		move.Flags = stream->ReadUInt32();

		int NumBoneIndices = stream->ReadIndex();
		for (int j = 0; j < NumBoneIndices; j++)
			move.BoneIndices.push_back(stream->ReadUInt32());

		int NumAnimTracks = stream->ReadIndex();
		for (int j = 0; j < NumAnimTracks; j++)
		{
			AnimTrack track;
			track.Flags = stream->ReadUInt32();

			int NumKeyQuat = stream->ReadIndex();
			for (int k = 0; k < NumKeyQuat; k++)
			{
				quaternion q;
				q.x = stream->ReadFloat();
				q.y = stream->ReadFloat();
				q.z = stream->ReadFloat();
				q.w = stream->ReadFloat();
				track.KeyQuat.push_back(q);
			}

			int NumKeyPos = stream->ReadIndex();
			for (int k = 0; k < NumKeyPos; k++)
			{
				vec3 pos;
				pos.x = stream->ReadFloat();
				pos.y = stream->ReadFloat();
				pos.z = stream->ReadFloat();
				track.KeyPos.push_back(pos);
			}

			int NumKeyTime = stream->ReadIndex();
			for (int k = 0; k < NumKeyTime; k++)
				track.KeyTime.push_back(stream->ReadFloat());

			move.AnimTracks.push_back(track);
		}

		AnimTrack rootTrack;
		rootTrack.Flags = stream->ReadUInt32();

		int NumKeyQuat = stream->ReadIndex();
		for (int k = 0; k < NumKeyQuat; k++)
		{
			quaternion q;
			q.x = stream->ReadFloat();
			q.y = stream->ReadFloat();
			q.z = stream->ReadFloat();
			q.w = stream->ReadFloat();
			rootTrack.KeyQuat.push_back(q);
		}

		int NumKeyPos = stream->ReadIndex();
		for (int k = 0; k < NumKeyPos; k++)
		{
			vec3 pos;
			pos.x = stream->ReadFloat();
			pos.y = stream->ReadFloat();
			pos.z = stream->ReadFloat();
			rootTrack.KeyPos.push_back(pos);
		}

		int NumKeyTime = stream->ReadIndex();
		for (int k = 0; k < NumKeyTime; k++)
			rootTrack.KeyTime.push_back(stream->ReadFloat());

		Moves.push_back(move);
	}
}
