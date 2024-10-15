using System.Collections.Generic;
using UnityEngine;

public class MineField : MonoBehaviour
{
    [SerializeField] private GameObject hazardObject;
    [SerializeField] private float hazardScale = 1f;
    [SerializeField] [Range(2f, 10f)] public float radius = 5;
    [SerializeField] private float yStartPosition = 5f;
    [SerializeField] private float xWidth = 100f;
    [SerializeField] private float yHeight = 10f;
    [SerializeField] private float zDepth = 100f;
    [SerializeField] private int rejectionSamples = 30;

    private List<Vector2> _points;

    void OnValidate()
    {
        _points = PoissonDiscSampling.GeneratePoints(radius, new Vector2(xWidth, zDepth), rejectionSamples);
    }

    void OnDrawGizmos()
    {
        Gizmos.color = Color.red;
        foreach (Vector2 point in _points)
        {
            Gizmos.DrawWireSphere(new Vector3(point.x - xWidth / 2, yStartPosition, point.y - zDepth / 2), 1);
        }
    }

    private void Start()
    {
        transform.position = new Vector3(transform.position.x, yStartPosition, transform.position.z);
        foreach (Vector2 point in _points) SpawnHazardObject(point);
    }

    private void SpawnHazardObject(Vector2 point)
    {
        var position = new Vector3(
            point.x - xWidth / 2,
            Random.Range(transform.position.y, transform.position.y + yHeight),
            point.y - zDepth / 2);

        GameObject go = Instantiate(hazardObject, position, Random.rotationUniform, transform);
        go.transform.localScale *= hazardScale;
    }
}