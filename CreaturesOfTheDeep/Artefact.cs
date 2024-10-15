using System.Collections.Generic;
using UnityEngine;

public class Artefact : MonoBehaviour
{
    public static List<Artefact> All { get; } = new();

    public int artID;
    public string artName;
    [TextArea] public string artDescription;

    private void OnEnable()
    {
        All.Add(this);
    }

    private void OnDisable()
    {
        All.Remove(this);
    }
}