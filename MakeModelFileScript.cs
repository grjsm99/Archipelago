using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.IO;
using System.Text;
using UnityEditor;
using Unity.VisualScripting;



public class MakeModelFileScript : MonoBehaviour
{
    void BinaryWriteString(string str, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(str.Length);
        binaryWriter.Write(str.ToCharArray());
    }
    void BinaryWriteVector3(Vector3 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
    }
    void BinaryWriteVector2(Vector2 vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
    }
    void BinaryWriteQuat(Quaternion vector, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(vector.x);
        binaryWriter.Write(vector.y);
        binaryWriter.Write(vector.z);
        binaryWriter.Write(vector.w);
    }

    void BinaryWriteColor(Color c, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(c.r);
        binaryWriter.Write(c.g);
        binaryWriter.Write(c.b);
        binaryWriter.Write(c.a);
    }
    void BinaryWriteMatrix(Matrix4x4 matrix, BinaryWriter binaryWriter)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }
    void BinaryWriteMaterial(Material material, BinaryWriter binaryWriter)
    {
        // materialNameSize(UINT) / materialName(string)

        BinaryWriteString(material.name, binaryWriter);
        
        // ambient(XMFLOAT4)
        Color ambient = new Color(0.2f, 0.2f, 0.2f, 1.0f);
        BinaryWriteColor(ambient, binaryWriter);
        // diffuse(XMFLOAT4)
        if (material.HasProperty("_Color"))
        {
            Color diffuse = material.GetColor("_Color");
            BinaryWriteColor(diffuse, binaryWriter);
        }
        else
        {
            Color diffuse = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(diffuse, binaryWriter);
        }
        // specular(XMFLOAT4)(specular.w = 반짝임계수)
        if (material.HasProperty("_SpecColor"))
        {
            Color specular = material.GetColor("_SpecColor");
            BinaryWriteColor(specular, binaryWriter);
        }
        else
        {
            Color specular = new Color(1.0f, 1.0f, 1.0f, 10.0f);
            BinaryWriteColor(specular, binaryWriter);
        }
        // emissive(XMFLOAT4)
        if (material.HasProperty("_EmissionColor"))
        {
            Color emission = material.GetColor("_EmissionColor");
            BinaryWriteColor(emission, binaryWriter);
        }
        else
        {
            Color emission = new Color(0.0f, 0.0f, 0.0f, 1.0f);
            BinaryWriteColor(emission, binaryWriter);
        }
    }
  
    string CreateMeshBinaryFile(Mesh mesh, MeshRenderer meshRenderer, string filePath, ref Bounds modelBound, Vector3 vec, string instanceName, string objName)
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open(filePath + "Mesh_" + instanceName + "_" + objName, FileMode.Create));

        // nVertex(UINT)
        // init modelbound
        
        Bounds worldBounds = new Bounds(mesh.bounds.center + vec, mesh.bounds.extents * 2);
        if(worldBounds.center.y > -20.0f) modelBound.Encapsulate(worldBounds);
        
        binaryWriter.Write((uint)mesh.vertexCount);
        // nameSize (UINT) / name (string)
        BinaryWriteString("Mesh_"  + objName, binaryWriter);
        // boundingBox (float * 6)
        BinaryWriteVector3(mesh.bounds.center, binaryWriter);
        BinaryWriteVector3(mesh.bounds.extents, binaryWriter);

        int id1 = 0, id2 = 0, id3 = 0;
        // positions (float * 3 * nVertex)
        foreach (Vector3 position in mesh.vertices)
        {
            BinaryWriteVector3(position, binaryWriter);
            id1++;
        }
        // normals (float * 3 * nVertex)
        foreach (Vector3 normal in mesh.normals)
        {
            BinaryWriteVector3(normal, binaryWriter);
            id2++;
        }
        
        // numTexcoord (INT)
        binaryWriter.Write((int)mesh.uv.Length);

        // texcoord (float * 2 * nVertex)
        foreach (Vector2 texcoord in mesh.uv)
        {
            Vector2 tmpuv = texcoord;
            tmpuv.y = 1 - tmpuv.y;  
            BinaryWriteVector2(tmpuv, binaryWriter);
            id3++;
        }
        // nSubMesh (UINT)
        binaryWriter.Write((uint)mesh.subMeshCount);

        for (int i = 0; i < mesh.subMeshCount; i++)
        {
            // nSubMeshIndex (UINT) / subMeshIndex (UINT * nSubMeshIndex)    ....( * nSubMesh )
            int[] subindicies = mesh.GetTriangles(i);
            binaryWriter.Write(subindicies.Length);
            
            foreach (int index in subindicies)
            {
                binaryWriter.Write(index);
            }
            Debug.Log(objName + " , " + mesh.subMeshCount + " , " + mesh.vertexCount + " , " + id1 + " , " + id2 + " , " + id3);
        }
        binaryWriter.Flush();
        binaryWriter.Close();
        return "Mesh_" + instanceName + "_" + objName;
    }
    void CreateObjectBinaryFile(Transform curObjectTransform, BinaryWriter binaryWriter, string filePath, ref Bounds modelBound, string instanceName)
    {
        string objectName = "GameObject_" + curObjectTransform.name;
        
        // nameSize (UINT) / name(string)
        BinaryWriteString(objectName, binaryWriter);

        //localPosition(float3)
        BinaryWriteVector3(curObjectTransform.localPosition, binaryWriter);

        //localScale(float3)
        BinaryWriteVector3(curObjectTransform.localScale, binaryWriter);

        //localRotation(float4)
        BinaryWriteQuat(curObjectTransform.localRotation, binaryWriter);

       
        MeshFilter meshFilter = curObjectTransform.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = curObjectTransform.GetComponent<MeshRenderer>();

        
        if (meshFilter && meshRenderer)    // 메쉬가 있는 경우
        {

            // meshNameSize(UINT) / meshName(string)	=> 메쉬가 없을 경우 따로 처리
            BinaryWriteString(CreateMeshBinaryFile(meshFilter.sharedMesh, meshRenderer, filePath, ref modelBound, curObjectTransform.position, instanceName, curObjectTransform.name), binaryWriter);

            // material 리스트 정보
            Material[] materialList = meshRenderer.sharedMaterials;
            
            // materialSize(UINT)
            binaryWriter.Write((uint)materialList.Length);

            foreach (Material mat in materialList)
            {
                BinaryWriteMaterial(mat, binaryWriter);
            }
            
        }
        else  // 메쉬가 없는 경우
        {
            binaryWriter.Write(0);
        }
        
        
        // nChildren(UINT)
        binaryWriter.Write(curObjectTransform.childCount);
        for (int i = 0; i < curObjectTransform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            // vec = worldPosition
            CreateObjectBinaryFile(curObjectTransform.GetChild(i), binaryWriter, filePath, ref modelBound, instanceName);
        }
    }

    void MakeModel(Transform _gameObject)
    {
        string fileName = _gameObject.name;
        Bounds modelBound = new Bounds();
        
        modelBound.size = Vector3.zero;
        
        DirectoryInfo directoryInfo = new DirectoryInfo("ModelBinaryFile/" + fileName);
        if (directoryInfo.Exists == false)
        {
            directoryInfo.Create();
        }
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("ModelBinaryFile/" + fileName + "/" + fileName, FileMode.Create));
        CreateObjectBinaryFile(_gameObject.transform, binaryWriter, "ModelBinaryFile/" + fileName + "/", ref modelBound, fileName);
        BinaryWriteVector3(modelBound.center, binaryWriter);
        BinaryWriteVector3(modelBound.extents, binaryWriter);
        Debug.Log("b : " + modelBound.center + " , " + modelBound.extents);
        binaryWriter.Flush();
        binaryWriter.Close();
    }
    // Start is called before the first frame update
    void Start()
    {
        for (int i = 0; i < transform.childCount; i++)  // 자식들을 똑같은 포멧으로 저장
        {
            MakeModel(transform.GetChild(i));
        }
    }

}
