using UnityEngine;
using UnityEngine.Events;

public class MineDetectorSystem : MonoBehaviour
{
    [SerializeField] private SphereCollider outerCollider;
    [SerializeField] private CapsuleCollider innerCollider;

    [SerializeField] public UnityEvent minesDetected;
    [SerializeField] public UnityEvent minesExited;
    [SerializeField] public UnityEvent minesExploded;

    private int _visibleMinesCount;

    private void OnTriggerEnter(Collider other)
    {
        if (!other.gameObject.TryGetComponent(out MineField _)) return;

        if (innerCollider.bounds.Intersects(other.bounds))
        {
            minesExploded.Invoke();
        }
        else if (outerCollider.bounds.Intersects(other.bounds))
        {
            minesDetected.Invoke();
        }
    }

    private void OnTriggerExit(Collider other)
    {
        if (!other.gameObject.TryGetComponent(out MineField _)) return;
        if (!outerCollider.bounds.Intersects(other.bounds))
        {
            minesExited.Invoke();
        }
    }

    public void DebugLogMinesDetected()
    {
        // Warnings should go of in the submarine
        Debug.Log("Mine field is close!");
    }

    public void DebugLogMinesExited()
    {
        // Warnings should stop
        Debug.Log("You're safe from the mines");
    }

    public void DebugLogMinesExploded()
    {
        // You die
        Debug.Log("You exploded!");
    }
}