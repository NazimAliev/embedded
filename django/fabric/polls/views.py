from django.shortcuts import render

# Create your views here.
from django.http import HttpResponse
from rest_framework.response import Response
from rest_framework.views import APIView
from .models import Question
from .models import Poller 
from .models import AnswerText 
from .models import AnswerChoice 
from .serializers import QuestionSerializer
from .serializers import PollerSerializer
from .serializers import AnswerTextSerializer 
from .serializers import AnswerChoiceSerializer 

def index(request):
	return HttpResponse("Hello, world. You're at the polls index.")

class PollerView(APIView):
	def get(self, request):
		poller = Poller.objects.all()
		serializer = PollerSerializer(poller, many=True)
		return Response({"poller" : serializer.data})

class QuestionView(APIView):
	def get(self, request):
		question = Question.objects.all()
		serializer = QuestionSerializer(question, many=True)
		return Response({"question" : serializer.data})

class AnswerTextView(APIView):
	def get(self, request, user_id):
		answer_text = AnswerText.objects.filter(user_id=user_id)
		serializer = AnswerTextSerializer(answer_text, many=True)
		return Response({"answer_text" : serializer.data})

	def post(self, request):
		answer_text = request.data.get('answer_text')
		serializer = AnswerTextSerializer(data=answer_text)
		if serializer.is_valid(raise_exception=True):
			answer_saved = serializer.save()
		return Response({"success": "Anser Text  '{}' created successfully".format(answer_saved)})

class AnswerChoiceView(APIView):
	def get(self, request, user_id):
		answer_choice = AnswerChoice.objects.filter(user_id=user_id)
		serializer = AnswerChoiceSerializer(answer_choice, many=True)
		return Response({"answer_choice" : serializer.data})

	def post(self, request):
		answer_choice = request.data.get('answer_choice')
		serializer = AnswerChoiceSerializer(data=answer_choice)
		if serializer.is_valid(raise_exception=True):
			answer_saved = serializer.save()
		return Response({"success": "Anser Choice  '{}' created successfully".format(answer_saved)})
